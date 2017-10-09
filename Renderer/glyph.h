#ifndef GLYPH_H
#define GLYPH_H

#include "visualization.h"

namespace vis
{
	/**
	 * @brief The Glyph class renders fields of normal distribution parameters using a 2D glyph based visualization.
	 */
	class Glyph : public Visualization
	{
	public:
		explicit Glyph(InputManager& input, const std::vector<Field>& fields);
		~Glyph() = default;

		virtual void setup_data() override;
		virtual void setup_shaders() override;

		virtual void update(float delta_time, float total_time) override;
		virtual void draw() const override;

		virtual glm::ivec2 point_under_cursor() const override;

	protected:
		std::vector<std::string> _vertex_shaders{"Shader/glyph_vs.glsl"};
		std::vector<std::string> _geometry_shaders{"Shader/glyph_gs.glsl"};
		std::vector<std::string> _fragment_shaders{"Shader/glyph_fs.glsl",
												   "Shader/palette.glsl"};
		// Data
		glm::vec2 _mean_bounds;	// (min, max)
		glm::vec2 _dev_bounds;	// (min, max)

	private:
		// Cursor
		Primitives _cursor_indicator{{{-1.f, -1.f, 0.f}, {1.f, 1.f, 0.f}, {-1.f, 1.f, 0.f}, {1.f, -1.f, 0.f}}};

		// Camera
		float _scale{1.f};
		glm::vec2 _translation{0.f};

		// Uniform locations
		GLint _mvp_loc{-1};
		GLint _bounds_loc{-1};
		GLint _fieldsize_loc{-1};
	};
}
#endif // GLYPH_H
