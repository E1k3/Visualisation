#ifndef HEIGHTFIELDRENDERER_H
#define HEIGHTFIELDRENDERER_H

#include <algorithm>
#include <string>
#include <chrono>

#include <glm/glm.hpp>

#include "renderer.h"
#include "Data/ensemblemanager.h"
#include "inputmanager.h"

namespace vis
{
	class HeightfieldRenderer : public Renderer
	{
	public:
		/// @brief HeightfieldRenderer Constructor.
		explicit HeightfieldRenderer(EnsembleManager& ensemble, InputManager& input);
		/// @brief HeightfieldRenderer Default copy constructor.
		explicit HeightfieldRenderer(const HeightfieldRenderer& other) = default;
		/// @brief HeightfieldRenderer Move constructor.
		explicit HeightfieldRenderer(HeightfieldRenderer&& other) noexcept;
		/// @brief operator = Copy assignment operator.
		HeightfieldRenderer& operator=(HeightfieldRenderer other) noexcept;
		/// @brief operator = Move assignment operator.
		HeightfieldRenderer& operator=(HeightfieldRenderer&& other) noexcept;

		virtual ~HeightfieldRenderer() = default;

		/**
		 * @brief swap Swaps state of two HeightfieldRenderers.
		 */
		friend void swap(HeightfieldRenderer& first, HeightfieldRenderer& second)
		{
			using std::swap;
			swap(static_cast<Renderer&>(first), static_cast<Renderer&>(second));

			swap(first._ensemble, second._ensemble);
			swap(first._input, second._input);

			swap(first._mvp_uniform, second._mvp_uniform);

			swap(first._cam_position, second._cam_position);
			swap(first._cam_direction, second._cam_direction);
		}

		void draw(float delta_time);

	private:
		EnsembleManager& _ensemble;
		InputManager& _input;

		int _mvp_uniform;

		glm::vec3 _cam_position{glm::vec3{1.8f}};
		glm::vec3 _cam_direction{-_cam_position};
	};
}

#endif // HEIGHTFIELDRENDERER_H
