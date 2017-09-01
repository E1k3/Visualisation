#ifndef GLYPH_H
#define GLYPH_H

#include "visualisation.h"

namespace vis
{
	class Glyph : public Visualisation
	{
	public:
		explicit Glyph(InputManager& input, const std::vector<Field>& fields);
		~Glyph() = default;

		void update(float delta_time, float total_time) override;

		void draw() const override;

	protected:
		void setup_data() override;
		void setup_shaders() override;

		glm::vec2 _mean_bounds;	// (min, max)
		glm::vec2 _dev_bounds;	// (min, max)



	private:
	};
}
#endif // GLYPH_H
