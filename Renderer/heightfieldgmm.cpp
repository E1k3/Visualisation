#include "heightfieldgmm.h"

#include "render_util.h"
#include "Data/math_util.h"

namespace vis
{
	HeightfieldGMM::HeightfieldGMM(InputManager& input, const std::vector<Field>& fields)
		: Heightfield{input, fields}
	{

	}

	void HeightfieldGMM::setup_data()
	{
		using namespace render_util;
		if(_fields.size() < 3)
		{
			Logger::error() << "HeightfieldGMM renderer needs at least two data fields to be created.";
			throw std::runtime_error("HeightfieldGMM renderer setup with < 2 fields");
		}
		if(!_fields[0].equal_layout(_fields[1]) || !_fields[0].equal_layout(_fields[2]))
		{
			Logger::error() << "HeightfieldGMM renderer needs data fields with matching format.";
			throw std::runtime_error("HeightfieldGMM renderer setup with invalid fields");
		}
		if(!(_fields[0].point_dimension() >= 4) || !(_fields[1].point_dimension() >= 4) || !(_fields[2].point_dimension() >= 4))
		{
			Logger::error() << "HeightfieldGMM renderer needs data fields with point depth >= 4.";
			throw std::runtime_error("HeightfieldGMM renderer setup with invalid fields");
		}

		auto& mean_field = _fields[0];	// Field holding the mean for each vertex
		auto& dev_field = _fields[1];	// Field holding the deviation for each vertex
		auto& weight_field = _fields[2];	// Field holding the GMM weights for each vertex
		// Vector holding the 2D position for each vertex
		auto grid = render_util::gen_grid(mean_field.width(), mean_field.height());

		glBindVertexArray(_vao = gen_vertex_array());

		// Setup VBO
		_buffers.push_back(gen_buffer());
		glBindBuffer(GL_ARRAY_BUFFER, _buffers.back());
		auto total_buffersize = grid.size() + 3*static_cast<size_t>(mean_field.area() * mean_field.point_dimension());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float) * total_buffersize), nullptr, GL_STATIC_DRAW);
		GLintptr buffer_offset = 0;

		// Vertex grid (position)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float)*grid.size()),	grid.data());
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		buffer_offset += grid.size() * sizeof(float);

		// Mean (ring)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float))*mean_field.area()*mean_field.point_dimension(), mean_field.data().data());
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, mean_field.point_dimension()*static_cast<int>(sizeof(float)), reinterpret_cast<void*>(buffer_offset));
		glEnableVertexAttribArray(1);
		buffer_offset += static_cast<GLsizeiptr>(sizeof(float))*mean_field.area()*mean_field.point_dimension();

		// Deviation (dot & background)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float))*dev_field.area()*dev_field.point_dimension(), dev_field.data().data());
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, dev_field.point_dimension()*static_cast<int>(sizeof(float)), reinterpret_cast<void*>(buffer_offset));
		glEnableVertexAttribArray(2);
		buffer_offset += static_cast<GLsizeiptr>(sizeof(float))*dev_field.area()*dev_field.point_dimension();

		// Weight (time)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float))*weight_field.area()*weight_field.point_dimension(), weight_field.data().data());
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, weight_field.point_dimension()*static_cast<int>(sizeof(float)), reinterpret_cast<void*>(buffer_offset));
		glEnableVertexAttribArray(3);
		buffer_offset += static_cast<GLsizeiptr>(sizeof(float))*weight_field.area()*weight_field.point_dimension();

		// Infices (element buffer)
		auto indices = render_util::gen_grid_indices(mean_field.width(), mean_field.height());
		_buffers.push_back(gen_buffer());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers.back());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned)*indices.size()), indices.data(), GL_STATIC_DRAW);

		// Set number of vertex indices to render
		_vertex_count = static_cast<int>(indices.size());

		// Set data bounds
		std::tie(_mean_bounds.x, _mean_bounds.y) = math_util::round_interval(mean_field.minimum(), mean_field.maximum());
		std::tie(_dev_bounds.x, _dev_bounds.y) = math_util::round_interval(dev_field.minimum(), dev_field.maximum());
	}

	void HeightfieldGMM::setup_shaders()
	{
		_vertex_shaders = {"/home/eike/Documents/Code/Visualisation/Shader/gmm_heightfield_vs.glsl"};
		_fragment_shaders = {"/home/eike/Documents/Code/Visualisation/Shader/gmm_heightfield_fs.glsl",
							 "/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"};

		Heightfield::setup_shaders();
	}
}
