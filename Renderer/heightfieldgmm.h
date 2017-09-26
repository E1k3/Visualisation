#ifndef HEIGHTFIELDGMM_H
#define HEIGHTFIELDGMM_H

#include "heightfield.h"

namespace vis
{
	/**
	 * @brief The HeightfieldGMM class renders fields of gaussian mixture model parameters using an animated 3D height field based visualization.
	 */
	class HeightfieldGMM : public Heightfield
	{
	public:
		explicit HeightfieldGMM(InputManager& input, const std::vector<Field>& fields);

		void setup_data() override;
		void setup_shaders() override;
	};
}

#endif // HEIGHTFIELDGMM_H
