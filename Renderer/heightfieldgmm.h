#ifndef HEIGHTFIELDGMM_H
#define HEIGHTFIELDGMM_H

#include "heightfield.h"

namespace vis
{
	class HeightfieldGMM : public Heightfield
	{
	public:
		explicit HeightfieldGMM(InputManager& input, const std::vector<Field>& fields);

		void setup_data() override;
		void setup_shaders() override;
	};
}

#endif // HEIGHTFIELDGMM_H
