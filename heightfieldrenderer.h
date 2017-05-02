#ifndef HEIGHTFIELDRENDERER_H
#define HEIGHTFIELDRENDERER_H

#include <algorithm>
#include <string>

#include "renderer.h"
#include "Data/ensemblemanager.h"

namespace vis
{
	class HeightfieldRenderer : public Renderer
	{
	public:
		/// @brief HeightfieldRenderer Constructor.
		explicit HeightfieldRenderer(EnsembleManager& ensemble);
		/// @brief HeightfieldRenderer Default copy constructor.
		explicit HeightfieldRenderer(HeightfieldRenderer& other) = default;
		/// @brief HeightfieldRenderer Move constructor.
		explicit HeightfieldRenderer(HeightfieldRenderer&& other) noexcept;
		/// @brief operator = Copy assignment operator.
		HeightfieldRenderer& operator=(HeightfieldRenderer other) noexcept;
		/// @brief operator = Move assignment operator.
		HeightfieldRenderer& operator=(HeightfieldRenderer&& other) noexcept;

		/**
		 * @brief swap Swaps state of two HeightfieldRenderers.
		 */
		friend void swap(HeightfieldRenderer& first, HeightfieldRenderer& second)
		{
			using std::swap;
			swap(static_cast<Renderer&>(first), static_cast<Renderer&>(second));
			swap(first._ensemble, second._ensemble);
		}

		void draw();

	private:
		EnsembleManager& _ensemble;
	};
}

#endif // HEIGHTFIELDRENDERER_H
