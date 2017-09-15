#ifndef GLYPHGMM_H
#define GLYPHGMM_H

#include "glyph.h"

namespace vis
{
	class GlyphGMM : public Glyph
	{
	public:
		explicit GlyphGMM(InputManager& input, const std::vector<Field>& fields, bool alternative_shader = false);

		void setup_data() override;
		void setup_shaders() override;
	};
}
#endif // GLYPHGMM_H
