#include "ensemble.h"

#include <exception>
#include <algorithm>
#include <fstream>
#include <string>

#include "logger.h"

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
			Logger::instance() << Logger::Severity::ERROR
							   << "Ensemble root directory does not contain subdirectories of the same size.\n"
							   << "Path: " << root;

			throw std::invalid_argument("Path does not follow the expected ensemble directory structure");
		}

		_num_steps = count_files(*fs::directory_iterator{root});
		_files.reserve(static_cast<size_t>(_num_simulations * _num_steps));

		// Copy all directories
		std::copy_if(fs::recursive_directory_iterator{root}, fs::recursive_directory_iterator{}, std::back_inserter(_files),
					 [] (const fs::path& p) { return fs::is_regular_file(p); });
		// Sort by path (simulation), then stably by name (timestep)
		std::sort(_files.begin(), _files.end());
		std::stable_sort(_files.begin(), _files.end(),
						 [] (const fs::path& a, const fs::path& b) { return a.filename() < b.filename(); });
	}

	int Ensemble::num_steps() const                    { return _num_steps; }

	int Ensemble::num_simulations() const              { return _num_simulations; }

	const std::vector<Field>& Ensemble::fields() const { return _fields; }

	void Ensemble::read_headers(int step_index)
	{
		if(step_index < 0 || step_index >= _num_steps)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Headers of step " << step_index << " cannot be read.\n"
							   << "Number of simulations: " << _num_simulations;

			throw std::out_of_range("Index of simulation step is out of range");
		}


		auto fields = std::vector<std::vector<Field>>(static_cast<size_t>(_num_simulations));
		for(int i = 0; i < _num_simulations; ++i)
		{
			auto& file = _files[ static_cast<size_t>(step_index * _num_simulations + i)];
			auto ifs = std::ifstream{file};
			auto buff = std::string{};
			auto line = std::stringstream{};

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
				Logger::instance() << Logger::Severity::ERROR
								   << "Field in file " << file
								   << " has invalid dimensions.\n"
								   << "width: " << width << " height: " << height << " depth: " << depth << " but total=" << total;

				throw std::runtime_error("Total size in simulation header is invalid");
			}

			// Advance one line
			std::getline(ifs, buff);
			line.str(buff);

			// Read number of fields
			std::getline(line, buff, ' ');
			fields[static_cast<size_t>(i)].resize(std::stoul(buff), Field(1, width, height, depth));
			// Read field names
			for(auto& field : fields[static_cast<size_t>(i)])
			{
				std::getline(line, buff, ' ');
				field.set_name(buff);
			}
		}

		// Compares two vectors of fields.
		// Returns true, if the vectors contain fields of differing layouts at the same index.
		auto not_equal = [](const auto& va, const auto& vb) { return !std::equal(va.begin(), va.end(), vb.begin(), [](const auto& fa, const auto& fb){ return fa.equal_layout(fb) && fa.name() == fb.name(); }); };
		if(std::adjacent_find(fields.begin(), fields.end(), not_equal) != fields.end())
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Ensemble contains fields of differing layout or name.";

			throw std::runtime_error("Field layout mismatch");
		}

		_fields.clear();
		_fields.reserve(fields.size());
		std::copy(fields.front().begin(), fields.front().end(), std::back_inserter(_fields));
	}

	int Ensemble::count_files(const fs::path& dir)
	{
		if(!fs::is_directory(dir))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Counting files failed.\n"
							   << dir.string() << " does not point to a directory.";
			throw std::invalid_argument("Path does not point to directory");	// TODO:ERROR handling. Dir is not a directory.
		}
		return static_cast<int>(std::count_if(fs::directory_iterator(dir), fs::directory_iterator{},
											  [] (const auto& path) { return fs::is_regular_file(path); }));
	}

	int Ensemble::count_directories(const fs::path& dir)
	{
		if(!fs::is_directory(dir))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Counting directories failed.\n"
							   << dir.string() << " does not point to a directory.";
			throw std::invalid_argument("Path does not point to directory");	// TODO:ERROR handling. Dir is not a directory.
		}
		return static_cast<int>(std::count_if(fs::directory_iterator(dir), fs::directory_iterator{},
											  [] (const auto& path) { return fs::is_directory(path); }));
	}
}
