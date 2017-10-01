#include "render_util.h"

#include <fstream>
#include <algorithm>

#include "logger.h"

namespace vis
{
	void render_util::load_compile_shader(GLuint id, const std::vector<std::string>& paths)
	{
		// Load code
		auto sources = std::vector<std::string>{};
		for(const auto& path : paths)
		{
			Logger::debug() << "Loading shader from source: " << path;
			sources.push_back("");
			auto ifs = std::ifstream(path);
			std::copy(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(),
					  std::back_inserter(sources.back()));
			sources.back().push_back('\0');
		}
		auto source_ptrs = std::vector<const char*>{};
		std::transform(sources.begin(), sources.end(), std::back_inserter(source_ptrs),
					   [] (const auto& s) { return s.c_str(); });

		glShaderSource(id, static_cast<GLsizei>(source_ptrs.size()), source_ptrs.data(), nullptr);
		glCompileShader(id);

		// Check for errors
		GLint success = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);

		if(success != GL_TRUE)
		{
			int logsize = 0;
			std::vector<char> log;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logsize);
			log.resize(static_cast<unsigned>(logsize));
			glGetShaderInfoLog(id, logsize, nullptr, log.data());

			Logger::error() << "Shader did not compile." << log.data();
			throw std::runtime_error("Load Shader Error");
		}
	}

	std::vector<float> render_util::gen_grid(int width, int height)
	{
		if(width < 0 || height < 0)
		{
			Logger::error() << "Grid generation using negative dimensions.\n"
							<< "width: " << width << " height: " << height;
			throw std::length_error("Negative grid generation dimensions");
		}


		auto grid = std::vector<float>(static_cast<size_t>(width * height * 2));

		if(grid.empty())
			return grid;

		for(int row = 0; row < height; ++row)
		{
			for(int col = 0; col < width; ++col)
			{
				float x = col/(width-1.f);
				float y = row/(height-1.f);
				grid[static_cast<size_t>(row*width + col)*2]    	= x*2.f - 1.f;
				grid[static_cast<size_t>(row*width + col)*2 + 1]	= y*2.f - 1.f;
			}
		}
		return grid;
	}

	std::vector<unsigned> render_util::gen_grid_indices(int width, int height)
	{
		if(width < 0 || height < 0)
		{
			Logger::error() << "Grid index generation using negative dimensions.\n"
							<< "width: " << width << " height: " << height;
			throw std::length_error("Negative grid index generation dimensions");
		}

		auto indices = std::vector<unsigned>{};
		if(width*height <= 2)
			return indices;

		for(int y = 0; y < height-1; ++y)
			for(int x = 0; x < width-1; ++x)
			{
				unsigned i = static_cast<unsigned>(y*width + x);
				indices.push_back(i+static_cast<unsigned>(width));
				indices.push_back(i+static_cast<unsigned>(width)+1);
				indices.push_back(i);
				indices.push_back(i+static_cast<unsigned>(width)+1);
				indices.push_back(i+1);
				indices.push_back(i);
			}
		return indices;
	}

	Texture& render_util::get_uniform_colormap_texture()
	{
		static Texture tex{create_colormap_texture({0.f,0.f,0.f,0.027065f,2.143e-05f,0.f,0.052054f,7.4728e-05f,0.f,0.071511f,0.00013914f,0.f,0.08742f,0.0002088f,0.f,0.10109f,0.00028141f,0.f,0.11337f,0.000356f,2.4266e-17f,0.12439f,0.00043134f,3.3615e-17f,0.13463f,0.00050796f,2.1604e-17f,0.14411f,0.0005856f,0.f,0.15292f,0.00070304f,0.f,0.16073f,0.0013432f,0.f,0.16871f,0.0014516f,0.f,0.17657f,0.0012408f,0.f,0.18364f,0.0015336f,0.f,0.19052f,0.0017515f,0.f,0.19751f,0.0015146f,0.f,0.20401f,0.0015249f,0.f,0.20994f,0.0019639f,0.f,0.21605f,0.002031f,0.f,0.22215f,0.0017559f,0.f,0.22808f,0.001546f,1.8755e-05f,0.23378f,0.0016315f,3.5012e-05f,0.23955f,0.0017194f,3.3352e-05f,0.24531f,0.0018097f,1.8559e-05f,0.25113f,0.0019038f,1.9139e-05f,0.25694f,0.0020015f,3.5308e-05f,0.26278f,0.0021017f,3.2613e-05f,0.26864f,0.0022048f,2.0338e-05f,0.27451f,0.0023119f,2.2453e-05f,0.28041f,0.0024227f,3.6003e-05f,0.28633f,0.0025363f,2.9817e-05f,0.29229f,0.0026532f,1.9559e-05f,0.29824f,0.0027747f,2.7666e-05f,0.30423f,0.0028999f,3.5752e-05f,0.31026f,0.0030279f,2.3231e-05f,0.31628f,0.0031599f,1.2902e-05f,0.32232f,0.0032974f,3.2915e-05f,0.32838f,0.0034379f,3.2803e-05f,0.33447f,0.0035819f,2.0757e-05f,0.34057f,0.003731f,2.3831e-05f,0.34668f,0.0038848f,3.502e-05f,0.35283f,0.0040418f,2.4468e-05f,0.35897f,0.0042032f,1.1444e-05f,0.36515f,0.0043708f,3.2793e-05f,0.37134f,0.0045418f,3.012e-05f,0.37756f,0.0047169f,1.4846e-05f,0.38379f,0.0048986f,2.796e-05f,0.39003f,0.0050848f,3.2782e-05f,0.3963f,0.0052751f,1.9244e-05f,0.40258f,0.0054715f,2.2667e-05f,0.40888f,0.0056736f,3.3223e-05f,0.41519f,0.0058798f,2.159e-05f,0.42152f,0.0060922f,1.8214e-05f,0.42788f,0.0063116f,3.2525e-05f,0.43424f,0.0065353f,2.2247e-05f,0.44062f,0.006765f,1.5852e-05f,0.44702f,0.0070024f,3.1769e-05f,0.45344f,0.0072442f,2.1245e-05f,0.45987f,0.0074929f,1.5726e-05f,0.46631f,0.0077499f,3.0976e-05f,0.47277f,0.0080108f,1.8722e-05f,0.47926f,0.0082789f,1.9285e-05f,0.48574f,0.0085553f,3.0063e-05f,0.49225f,0.0088392f,1.4313e-05f,0.49878f,0.0091356f,2.3404e-05f,0.50531f,0.0094374f,2.8099e-05f,0.51187f,0.0097365f,6.4695e-06f,0.51844f,0.010039f,2.5791e-05f,0.52501f,0.010354f,2.4393e-05f,0.53162f,0.010689f,1.6037e-05f,0.53825f,0.011031f,2.7295e-05f,0.54489f,0.011393f,1.5848e-05f,0.55154f,0.011789f,2.3111e-05f,0.55818f,0.012159f,2.5416e-05f,0.56485f,0.012508f,1.5064e-05f,0.57154f,0.012881f,2.541e-05f,0.57823f,0.013283f,1.6166e-05f,0.58494f,0.013701f,2.263e-05f,0.59166f,0.014122f,2.3316e-05f,0.59839f,0.014551f,1.9432e-05f,0.60514f,0.014994f,2.4323e-05f,0.6119f,0.01545f,1.3929e-05f,0.61868f,0.01592f,2.1615e-05f,0.62546f,0.016401f,1.5846e-05f,0.63226f,0.016897f,2.0838e-05f,0.63907f,0.017407f,1.9549e-05f,0.64589f,0.017931f,2.0961e-05f,0.65273f,0.018471f,2.0737e-05f,0.65958f,0.019026f,2.0621e-05f,0.66644f,0.019598f,2.0675e-05f,0.67332f,0.020187f,2.0301e-05f,0.68019f,0.020793f,2.0029e-05f,0.68709f,0.021418f,2.0088e-05f,0.69399f,0.022062f,1.9102e-05f,0.70092f,0.022727f,1.9662e-05f,0.70784f,0.023412f,1.7757e-05f,0.71478f,0.024121f,1.8236e-05f,0.72173f,0.024852f,1.4944e-05f,0.7287f,0.025608f,2.0245e-06f,0.73567f,0.02639f,1.5013e-07f,0.74266f,0.027199f,0.f,0.74964f,0.028038f,0.f,0.75665f,0.028906f,0.f,0.76365f,0.029806f,0.f,0.77068f,0.030743f,0.f,0.77771f,0.031711f,0.f,0.78474f,0.032732f,0.f,0.79179f,0.033741f,0.f,0.79886f,0.034936f,0.f,0.80593f,0.036031f,0.f,0.81299f,0.03723f,0.f,0.82007f,0.038493f,0.f,0.82715f,0.039819f,0.f,0.83423f,0.041236f,0.f,0.84131f,0.042647f,0.f,0.84838f,0.044235f,0.f,0.85545f,0.045857f,0.f,0.86252f,0.047645f,0.f,0.86958f,0.049578f,0.f,0.87661f,0.051541f,0.f,0.88365f,0.053735f,0.f,0.89064f,0.056168f,0.f,0.89761f,0.058852f,0.f,0.90451f,0.061777f,0.f,0.91131f,0.065281f,0.f,0.91796f,0.069448f,0.f,0.92445f,0.074684f,0.f,0.93061f,0.08131f,0.f,0.93648f,0.088878f,0.f,0.94205f,0.097336f,0.f,0.9473f,0.10665f,0.f,0.9522f,0.1166f,0.f,0.95674f,0.12716f,0.f,0.96094f,0.13824f,0.f,0.96479f,0.14963f,0.f,0.96829f,0.16128f,0.f,0.97147f,0.17303f,0.f,0.97436f,0.18489f,0.f,0.97698f,0.19672f,0.f,0.97934f,0.20846f,0.f,0.98148f,0.22013f,0.f,0.9834f,0.23167f,0.f,0.98515f,0.24301f,0.f,0.98672f,0.25425f,0.f,0.98815f,0.26525f,0.f,0.98944f,0.27614f,0.f,0.99061f,0.28679f,0.f,0.99167f,0.29731f,0.f,0.99263f,0.30764f,0.f,0.9935f,0.31781f,0.f,0.99428f,0.3278f,0.f,0.995f,0.33764f,0.f,0.99564f,0.34735f,0.f,0.99623f,0.35689f,0.f,0.99675f,0.3663f,0.f,0.99722f,0.37556f,0.f,0.99765f,0.38471f,0.f,0.99803f,0.39374f,0.f,0.99836f,0.40265f,0.f,0.99866f,0.41145f,0.f,0.99892f,0.42015f,0.f,0.99915f,0.42874f,0.f,0.99935f,0.43724f,0.f,0.99952f,0.44563f,0.f,0.99966f,0.45395f,0.f,0.99977f,0.46217f,0.f,0.99986f,0.47032f,0.f,0.99993f,0.47838f,0.f,0.99997f,0.48638f,0.f,1.f,0.4943f,0.f,1.f,0.50214f,0.f,1.f,0.50991f,1.2756e-05f,1.f,0.51761f,4.5388e-05f,1.f,0.52523f,9.6977e-05f,1.f,0.5328f,0.00016858f,1.f,0.54028f,0.0002582f,1.f,0.54771f,0.00036528f,1.f,0.55508f,0.00049276f,1.f,0.5624f,0.00063955f,1.f,0.56965f,0.00080443f,1.f,0.57687f,0.00098902f,1.f,0.58402f,0.0011943f,1.f,0.59113f,0.0014189f,1.f,0.59819f,0.0016626f,1.f,0.60521f,0.0019281f,1.f,0.61219f,0.0022145f,1.f,0.61914f,0.0025213f,1.f,0.62603f,0.0028496f,1.f,0.6329f,0.0032006f,1.f,0.63972f,0.0035741f,1.f,0.64651f,0.0039701f,1.f,0.65327f,0.0043898f,1.f,0.66f,0.0048341f,1.f,0.66669f,0.005303f,1.f,0.67336f,0.0057969f,1.f,0.67999f,0.006317f,1.f,0.68661f,0.0068648f,1.f,0.69319f,0.0074406f,1.f,0.69974f,0.0080433f,1.f,0.70628f,0.0086756f,1.f,0.71278f,0.0093486f,1.f,0.71927f,0.010023f,1.f,0.72573f,0.010724f,1.f,0.73217f,0.011565f,1.f,0.73859f,0.012339f,1.f,0.74499f,0.01316f,1.f,0.75137f,0.014042f,1.f,0.75772f,0.014955f,1.f,0.76406f,0.015913f,1.f,0.77039f,0.016915f,1.f,0.77669f,0.017964f,1.f,0.78298f,0.019062f,1.f,0.78925f,0.020212f,1.f,0.7955f,0.021417f,1.f,0.80174f,0.02268f,1.f,0.80797f,0.024005f,1.f,0.81418f,0.025396f,1.f,0.82038f,0.026858f,1.f,0.82656f,0.028394f,1.f,0.83273f,0.030013f,1.f,0.83889f,0.031717f,1.f,0.84503f,0.03348f,1.f,0.85116f,0.035488f,1.f,0.85728f,0.037452f,1.f,0.8634f,0.039592f,1.f,0.86949f,0.041898f,1.f,0.87557f,0.044392f,1.f,0.88165f,0.046958f,1.f,0.88771f,0.04977f,1.f,0.89376f,0.052828f,1.f,0.8998f,0.056209f,1.f,0.90584f,0.059919f,1.f,0.91185f,0.063925f,1.f,0.91783f,0.068579f,1.f,0.92384f,0.073948f,1.f,0.92981f,0.080899f,1.f,0.93576f,0.090648f,1.f,0.94166f,0.10377f,1.f,0.94752f,0.12051f,1.f,0.9533f,0.14149f,1.f,0.959f,0.1672f,1.f,0.96456f,0.19823f,1.f,0.96995f,0.23514f,1.f,0.9751f,0.2786f,1.f,0.97992f,0.32883f,1.f,0.98432f,0.38571f,1.f,0.9882f,0.44866f,1.f,0.9915f,0.51653f,1.f,0.99417f,0.58754f,1.f,0.99625f,0.65985f,1.f,0.99778f,0.73194f,1.f,0.99885f,0.80259f,1.f,0.99953f,0.87115f,1.f,0.99989f,0.93683f,1.f,1.f,1.f,})};
		return tex;
	}

	Texture render_util::create_colormap_texture(const std::vector<float>& data)
	{
		auto texture = gen_texture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, texture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, static_cast<GLsizei>(data.size())/3, 0, GL_RGB, GL_FLOAT, data.data());
		glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		return texture;
	}

	VertexArray render_util::gen_vertex_array()
	{
		GLuint id = 0;
		glGenVertexArrays(1, &id);
		return VertexArray(id);
	}

	Buffer render_util::gen_buffer()
	{
		GLuint id = 0;
		glGenBuffers(1, &id);
		return Buffer(id);
	}

	Texture render_util::gen_texture()
	{
		GLuint id = 0;
		glGenTextures(1, &id);
		return Texture(id);
	}

	Program render_util::gen_program()
	{
		return Program{glCreateProgram()};
	}

	Shader render_util::gen_shader(GLenum shadertype)
	{
		return Shader(glCreateShader(shadertype));
	}
}
