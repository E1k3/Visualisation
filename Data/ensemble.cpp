#include "ensemble.h"

#include <exception>
#include <algorithm>
#include <fstream>
#include <string>
#include <cmath>

#include <thread>


#include "logger.h"
#include "math_util.h"

namespace vis
{
	Ensemble::Ensemble(const fs::path& root)
	{
		_num_simulations = count_directories(root);
		auto not_equal = [] (const auto& a, const auto& b) { return count_files(a) != count_files(b); };
		if(_num_simulations <= 0
				|| std::adjacent_find(fs::directory_iterator{root}, fs::directory_iterator{}, not_equal)
				!= fs::directory_iterator{})	// Search for a subdirectory of root with amount of files not equal to the others
		{
			Logger::error() << "Ensemble root directory does not contain subdirectories of the same size. "
							<< "Path: " << root;

			throw std::invalid_argument("Path does not follow the expected ensemble directory structure");
		}

		_num_steps = count_files(*fs::directory_iterator{root});
		_project_files.reserve(static_cast<size_t>(_num_simulations * _num_steps));	// Class invariant, has to be >= 0

		// Copy all directories
		std::copy_if(fs::recursive_directory_iterator{root}, fs::recursive_directory_iterator{}, std::back_inserter(_project_files),
					 [] (const fs::path& p) { return fs::is_regular_file(p); });
		// Sort by path (simulation), then stably by name (timestep)
		std::sort(_project_files.begin(), _project_files.end());
		std::stable_sort(_project_files.begin(), _project_files.end(),
						 [] (const fs::path& a, const fs::path& b) { return a.filename() < b.filename(); });
	}

	int Ensemble::num_steps() const						{ return _num_steps; }

	int Ensemble::num_simulations() const				{ return _num_simulations; }

	const std::vector<Field>& Ensemble::fields() const	{ return _fields; }

	const std::vector<Field>& Ensemble::headers() const	{ return _headers; }

	void Ensemble::read_headers(int step_index, int count, int stride)
	{
		if(step_index < 0 || step_index >= _num_steps)
		{
			Logger::error() << "Headers of step " << step_index << " cannot be read. "
							<< "Number of simulations: " << _num_simulations;

			throw std::out_of_range("Index of simulation step is out of range");
		}
		if(count < 0 || stride < 0 || stride > _num_steps || step_index + count * stride > _num_steps)
		{
			Logger::error() << "Headers of step " << step_index << " cannot be read. "
							<< "Aggregation count or stride are out of range: steps:" << _num_steps
							<< " count: " << count << " stride: " << stride;

			throw std::out_of_range("Index of simulation step is out of range");
		}
		if(count == 0)
			count = 1;
		if(stride == 0)
			stride = 1;


		auto fields = std::vector<std::vector<Field>>(static_cast<size_t>(_num_simulations * count));
		for(int c = 0; c < count; ++c)
		{
			for(int i = 0; i < _num_simulations; ++i)
			{
				auto& file = _project_files[static_cast<size_t>((step_index + c * stride) * _num_simulations + i)];
				auto ifs = std::ifstream{file};
				auto buff = std::string{};
				auto line = std::istringstream{};

				// Advance one line
				std::getline(ifs, buff);
				line.str(buff);

				// Read layout
				std::getline(line, buff, ' ');
				auto width = std::stoi(buff);
				std::getline(line, buff, ' ');
				auto height = std::stoi(buff);
				std::getline(line, buff, ' ');
				auto depth = std::stoi(buff);

				// Check if total points = volume
				std::getline(line, buff, ' ');
				auto total = std::stoi(buff);
				if(total != width*height*depth)
				{
					Logger::error() << "Field in file " << file
									<< " has invalid dimensions: "
									<< "width: " << width << " height: " << height << " depth: " << depth << " total:" << total;

					throw std::runtime_error("Total size in simulation header is invalid");
				}

				// Advance one line
				std::getline(ifs, buff);
				line.str(buff);

				// Read number of fields
				std::getline(line, buff, ' ');
				fields[static_cast<size_t>(_num_simulations * c + i)].resize(std::stoul(buff), Field(1, width, height, depth));
				// Read field names
				for(auto& field : fields[static_cast<size_t>(_num_simulations * c + i)])
				{
					std::getline(line, buff, ' ');
					field.set_name(buff);
				}
			}
		}

		// Compares two vectors of fields.
		// Returns true, if the vectors contain fields of differing layouts at the same index.
		auto not_equal = [](const auto& va, const auto& vb) { return !std::equal(va.begin(), va.end(), vb.begin(), [](const auto& fa, const auto& fb){ return fa.equal_layout(fb) && fa.name() == fb.name(); }); };
		if(std::adjacent_find(fields.begin(), fields.end(), not_equal) != fields.end())
		{
			Logger::error() << "Ensemble contains fields of differing layout or name.";

			throw std::runtime_error("Field layout mismatch");
		}

		_selected_step = step_index;
		_cluster_size = count;
		_cluster_stride = stride;
		_headers.clear();
		_headers.reserve(fields.size());
		std::copy(fields.front().begin(), fields.front().end(), std::back_inserter(_headers));
	}

	void Ensemble::analyse_field(int field_index, Ensemble::Analysis analysis)
	{
		if(field_index < 0 || static_cast<size_t>(field_index) >= _headers.size())
		{
			Logger::error() << "Analysing field failed, field at index " << field_index << " does not exist. "
							<< "Number of fields: " << _headers.size();
			throw std::invalid_argument("No field exists at index.");
		}

		// Read fields from file
		const auto& layout = _headers[static_cast<size_t>(field_index)];
		auto fields = std::vector<Field>(static_cast<size_t>(_num_simulations * _cluster_size), Field(layout, true));

		for(int c = 0; c < _cluster_size; ++c)
		{
			for(int i = 0; i < _num_simulations; ++i)
			{
				auto ifs = std::ifstream(_project_files[ static_cast<size_t>((_selected_step + c * _cluster_stride) * _num_simulations + i)]);
				ignore_many(ifs, 3, '\n');	// Skip header
				ignore_many(ifs, (layout.height()*layout.depth()+1)*field_index, '\n');	// Skip fields

				// Read data
				auto buff = std::string{};

				for(int j = 0; j < layout.volume(); ++j)
				{
					std::getline(ifs, buff, ' ');
					fields[static_cast<size_t>(c * _num_simulations + i)].set_value(0, j, std::stof(buff));
				}

				Logger::debug() << "Field " << fields[static_cast<size_t>(c * _num_simulations + i)].name() << " has been read successfully from file "
								<< _project_files[ static_cast<size_t>((_selected_step + c * _cluster_stride) * _num_simulations + i)];
			}
		}

		// Start analysis
		switch(analysis)
		{
		case Analysis::GAUSSIAN_SINGLE:
			_fields = gaussian_analysis(fields);
			break;
		case Analysis::GAUSSIAN_MIXTURE:
			_fields = gaussian_mixture_analysis(fields);
			break;
		}
	}

	void Ensemble::ignore_many(std::istream& stream, int count, char delimiter)
	{
		for(int i = 0; i < count; ++i)
			stream.ignore(std::numeric_limits<std::streamsize>::max(), delimiter);
	}

	std::vector<Field> Ensemble::gaussian_analysis(const std::vector<Field>& fields)
	{
		if(fields.empty())
		{
			Logger::error() << "No data for gaussian analysis.";
			throw std::invalid_argument("Missing data for gaussian analysis");
		}
		const auto& layout = fields.front();
		auto result = std::vector<Field>(2, Field(1, layout.width(), layout.height(), layout.depth(), true));
		result[0].set_name(layout.name() + "_mean");
		result[1].set_name(layout.name() + "_deviation");

		// Set multithreading to maximum hardware concurrency
		auto thread_count = std::min(std::max(static_cast<int>(std::thread::hardware_concurrency()), 1), result.front().volume());
		auto threads = std::vector<std::thread>();
		for(int t = 0; t < thread_count; ++t)
		{
			int start = result.front().volume() * t / thread_count;
			int end = result.front().volume() * (t+1) / thread_count;
			threads.emplace_back([&fields, &result, start, end] ()
			{
				for(int i = start; i < end; ++i)
				{
					auto samples = std::vector<float>();
					samples.reserve(fields.size());
					for(const auto& field : fields)
						samples.push_back(field.get_value(0, i));
					result[0].set_value(0, i, math_util::mean(samples));
					result[1].set_value(0, i, std::sqrt(math_util::variance(samples, result[0].get_value(0, i))));
				}
			});
		}

		for(auto& thread : threads)
			thread.join();

		Logger::debug() << "Fields " << result[0].name() << " and "<< result[1].name() << " have been calculated successfully.";

		return result;
	}

	std::vector<Field> Ensemble::gaussian_mixture_analysis(const std::vector<Field>& fields)
	{
		constexpr int gmm_components = 4;

		if(fields.empty())
		{
			Logger::error() << "No data for gmm analysis.";
			throw std::invalid_argument("Missing data for gmm analysis");
		}
		const auto& layout = fields.front();
		auto result = std::vector<Field>(3, Field(gmm_components, layout.width(), layout.height(), layout.depth(), true));
		result[0].set_name(layout.name() + "_mean");
		result[1].set_name(layout.name() + "_deviation");
		result[2].set_name(layout.name() + "_weight");

		// Set multithreading to maximum hardware concurrency
		auto thread_count = std::min(std::max(static_cast<int>(std::thread::hardware_concurrency()), 1), result.front().volume());
		auto threads = std::vector<std::thread>();
		for(int t = 0; t < thread_count; ++t)
		{
			int start = result.front().volume() * t / thread_count;
			int end = result.front().volume() * (t+1) / thread_count;
			threads.emplace_back([&fields, &result, start, end] ()
			{
				for(int i = start; i < end; ++i)
				{
					auto samples = std::vector<float>();
					samples.reserve(fields.size());
					for(const auto& field : fields)
						samples.push_back(field.get_value(0, i));

					auto gmm = math_util::fit_gmm(samples, gmm_components);
					for(int c = 0; c < gmm_components; ++c)
					{
						//				result[0].set_value(c, i, math_util::find_max(samples, gmm[static_cast<size_t>(c)]));
						//				result[0].set_value(c, i, math_util::find_median(samples, gmm[static_cast<size_t>(c)]));
						result[0].set_value(c, i, gmm[static_cast<size_t>(c)]._mean);
						result[1].set_value(c, i, std::sqrt(gmm[static_cast<size_t>(c)]._variance));
						result[2].set_value(c, i, gmm[static_cast<size_t>(c)]._weight);
					}
				}
			});
		}

		for(auto& thread : threads)
			thread.join();

		Logger::debug() << "Fields " << result[0].name()
						<< ", "<< result[1].name()
						<< " and "<< result[2].name()
						<< " have been calculated successfully.";

		return result;
	}

	int Ensemble::count_files(const fs::path& dir)
	{
		if(!fs::is_directory(dir))
		{
			Logger::error() << "Counting files failed. "
							<< dir.string() << " does not point to a directory.";
			throw std::invalid_argument("Path does not point to directory");
		}
		return static_cast<int>(std::count_if(fs::directory_iterator(dir), fs::directory_iterator{},
											  [] (const auto& path) { return fs::is_regular_file(path); }));
	}

	int Ensemble::count_directories(const fs::path& dir)
	{
		if(!fs::is_directory(dir))
		{
			Logger::error() << "Counting directories failed. "
							<< dir.string() << " does not point to a directory.";
			throw std::invalid_argument("Path does not point to directory");
		}
		return static_cast<int>(std::count_if(fs::directory_iterator(dir), fs::directory_iterator{},
											  [] (const auto& path) { return fs::is_directory(path); }));
	}
}
