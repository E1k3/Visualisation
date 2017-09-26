#ifndef GLYPHGMM_H
#define GLYPHGMM_H

#include "glyph.h"

namespace vis
{
	/**
	 * @brief The GlyphGMM class renders fields of gaussian mixture model parameters using a 2D glyph based visualization.
	 */
	class GlyphGMM : public Glyph
	{
	public:
		/**
		 * @param alternative_shader If true, renders the data using a different shader, resulting in a different visualisation of the same concept.
		 */
		explicit GlyphGMM(InputManager& input, const std::vector<Field>& fields, bool alternative_shader = false);

		void setup_data() override;
		void setup_shaders() override;
	};
}
#endif // GLYPHGMM_H
