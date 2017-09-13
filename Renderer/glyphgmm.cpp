#include "glyphgmm.h"

#include "render_util.h"
#include "Data/math_util.h"

namespace vis
{
	GlyphGMM::GlyphGMM(InputManager& input, const std::vector<Field>& fields)
		: Glyph{input, fields}
	{

	}

	void GlyphGMM::setup_data()
	{
		using namespace render_util;

		if(_fields.size() < 3)
		{
			Logger::error() << "GlyphGMM renderer needs at least two data fields to be created.";
			throw std::runtime_error("GlyphGMM renderer setup with < 2 fields");
		}
		if(!_fields[0].equal_layout(_fields[1]) || !_fields[0].equal_layout(_fields[2]))
		{
			Logger::error() << "GlyphGMM renderer needs data fields with matching format.";
			throw std::runtime_error("GlyphGMM renderer setup with invalid fields");
		}
		if(!(_fields[0].point_dimension() >= 4) || !(_fields[1].point_dimension() >= 4) || !(_fields[2].point_dimension() >= 4))
		{
			Logger::error() << "GlyphGMM renderer needs data fields with point depth >= 4.";
			throw std::runtime_error("GlyphGMM renderer setup with invalid fields");
		}

		auto& mean_field = _fields[0];	// Field holding the GMM means for each vertex
		auto& dev_field = _fields[1];	// Field holding the GMM deviations for each vertex
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

		// Weight (size)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float))*weight_field.area()*weight_field.point_dimension(), weight_field.data().data());
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, weight_field.point_dimension()*static_cast<int>(sizeof(float)), reinterpret_cast<void*>(buffer_offset));
		glEnableVertexAttribArray(3);
		buffer_offset += static_cast<GLsizeiptr>(sizeof(float))*weight_field.area()*weight_field.point_dimension();

		// Set number of vertices to render
		_vertex_count = mean_field.area();

		// Set data bounds
		_mean_bounds = glm::vec2(math_util::combined_minimum(mean_field, dev_field), math_util::combined_maximum(mean_field, dev_field));
		_dev_bounds = glm::vec2(0, _mean_bounds.y - _mean_bounds.x);

		_palette.set_bounds(_mean_bounds, 15);
	}

	void GlyphGMM::setup_shaders()
	{
		_vertex_shaders = {"/home/eike/Documents/Code/Visualisation/Shader/gmm_glyph_vs.glsl"};
		_geometry_shaders = {"/home/eike/Documents/Code/Visualisation/Shader/gmm_glyph_gs.glsl"};
		_fragment_shaders = {"/home/eike/Documents/Code/Visualisation/Shader/gmm_glyph_fs_2.glsl",
							 "/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"};
		Glyph::setup_shaders();
	}
}
