#ifndef HEIGHTFIELDRENDERER_H
#define HEIGHTFIELDRENDERER_H

#include <algorithm>
#include <string>
#include <chrono>

#include <glm/glm.hpp>

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
		explicit HeightfieldRenderer(const HeightfieldRenderer& other) = default;
		/// @brief HeightfieldRenderer Move constructor.
		explicit HeightfieldRenderer(HeightfieldRenderer&& other) noexcept;
		/// @brief operator = Copy assignment operator.
		HeightfieldRenderer& operator=(HeightfieldRenderer other) noexcept;
		/// @brief operator = Move assignment operator.
		HeightfieldRenderer& operator=(HeightfieldRenderer&& other) noexcept;

		virtual ~HeightfieldRenderer() = default;

		void setMVP(const glm::mat4& mvp) const;

		/**
		 * @brief swap Swaps state of two HeightfieldRenderers.
		 */
		friend void swap(HeightfieldRenderer& first, HeightfieldRenderer& second)
		{
			using std::swap;
			swap(static_cast<Renderer&>(first), static_cast<Renderer&>(second));
			swap(first._ensemble, second._ensemble);
			swap(first._mvp_uniform, second._mvp_uniform);
		}

		void draw();

	private:
		EnsembleManager& _ensemble;

		int _mvp_uniform;
	};
}

#endif // HEIGHTFIELDRENDERER_H
