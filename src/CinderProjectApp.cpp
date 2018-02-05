#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Params/Params.h"
#include "cinder/ImageIo.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// Home many particles to create. (200k)
const int MAX_PARTICLES = 200e3;

struct Particle
{
	vec3		pos;
	ColorA		color;
	float       radius;
};


class AttractorState
{
public:
	int iteration;
	float x;
	float y;

private:
	float a, b, c, d;

public:
	AttractorState(float in_a, float in_b, float in_c, float in_d)
	: a(in_a), b(in_b), c(in_c), d(in_d)
	{
		x = 0.1f;
		y = 0.1f;
		iteration = 0;
	}

	AttractorState& operator++()
	{
 		float xn = d * sin(a * x) - sin(b * y);
 		float yn = c * cos(a * x) + cos(b * y);
		x = xn;
 		y = yn;		
		++iteration;
		return *this;
 	};
};

class AttractorExplorer : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void button();

	void evaluateAttractor();

	// Actual partiicle positions
	std::vector<glm::vec2> mPoints;

	// Particle data on CPU.
	vector<Particle>	mParticles;

	// Particle system parameeters
	params::InterfaceGlRef	mParams;

	// Particle information
	unique_ptr<AttractorState> mState;

	// Particle colour
 	ColorA mParticleColor;

	// Buffer holding raw particle data on GPU, written to every update().
	gl::VboRef mParticleVbo;

	// Batch for rendering particles with default shader.
	gl::BatchRef mParticleBatch;

	// particle texture
	gl::TextureRef mTexture;

	// particle shader
	gl::GlslProgRef mPointShader;

	float a = 1.40f; 
	float b = 1.56f; 
	float c = 1.40f; 
	float d = -6.5f;

	int moreParticles = 5000;
};


void AttractorExplorer::setup()
{
	try
	{
		mPointShader = gl::GlslProg::create(loadAsset("shader.vert"), loadAsset("shader.frag"));
		CI_LOG_I("Loaded shader");
	}
	catch (const std::exception& e) {
		CI_LOG_E("Shader Error: " << e.what());
	}

	try {
		mTexture = gl::Texture::create(loadImage(loadAsset("particle.png")), gl::Texture::Format().mipmap());
		//mTexture->bind();
		CI_LOG_I("Loaded texture");
	}
	catch (const std::exception& e) {
		CI_LOG_E("Texture Error: " << e.what());
	}

	// set up particles
	mParticleColor = ColorA(0.9f, 1.0f, 0.3f, 0.7f);
	mState = make_unique<AttractorState>(a, b, c, d);

	// Create initial particle layout.
	mParticles.assign(MAX_PARTICLES, Particle());
	mPoints.assign(MAX_PARTICLES, glm::vec2());

	// Create the interface and give it a name.
	mParams = params::InterfaceGl::create(getWindow(), "Parameters", toPixels(ivec2(200, 300)));

	// Setup some basic parameters.
	mParams->addParam("A", &a).min(-10.0f).max(10.0f).keyIncr("a").keyDecr("A").precision(3).step(0.001f);
	mParams->addParam("B", &b).min(-10.0f).max(10.0f).keyIncr("b").keyDecr("B").precision(3).step(0.001f);
	mParams->addParam("C", &c).min(-10.0f).max(10.0f).keyIncr("c").keyDecr("C").precision(3).step(0.001f);
	mParams->addParam("D", &d).min(-10.0f).max(10.0f).keyIncr("d").keyDecr("D").precision(3).step(0.001f);
	mParams->addParam("Color", &mParticleColor);
	mParams->addParam("Count", &mState->iteration, true);
	mParams->addSeparator();
	mParams->addButton("Restart", bind(&AttractorExplorer::button, this));


	// Create particle buffer on GPU and copy over data.
	// Mark as streaming, since we will copy new data every frame.
	mParticleVbo = gl::Vbo::create(GL_ARRAY_BUFFER, mParticles, GL_STREAM_DRAW);

	// Describe particle semantics for GPU.
	geom::BufferLayout particleLayout;
	particleLayout.append(geom::Attrib::POSITION, 3, sizeof(Particle), offsetof(Particle, pos));
	particleLayout.append(geom::Attrib::COLOR, 4, sizeof(Particle), offsetof(Particle, color));
	particleLayout.append(geom::Attrib::CUSTOM_0, 1, sizeof(Particle), offsetof(Particle, radius));

	gl::enable(GL_VERTEX_PROGRAM_POINT_SIZE, true);
	// Create mesh by pairing our particle layout with our particle Vbo.
	// A VboMesh is an array of layout + vbo pairs
	auto mesh = gl::VboMesh::create(mParticles.size(), GL_POINTS, { { particleLayout, mParticleVbo } });
	mParticleBatch = gl::Batch::create(mesh, mPointShader, { { geom::Attrib::CUSTOM_0, "particleRadius" } });
}

void AttractorExplorer::button()
{
	console() << "Clicked!" << endl;
	mParams->setOptions( "text", "label=`Clicked!`" );
}


void AttractorExplorer::evaluateAttractor()
{
	if ((mState->iteration < MAX_PARTICLES) && (mState->iteration < moreParticles))
	{
		vec2 size = vec2( getWindowSize()  );
		vec2 center = vec2( getWindowCenter()  );
		vec2 scale = size / 17.0f;
		vec2 point = glm::vec2(mState->x, mState->y);
		//console() << "Point " << mState->iteration << " is " << point << "\n";
		mPoints[mState->iteration] = point;
 		vec2 pixel = (point * scale) + center;
		mParticles[mState->iteration].pos = vec3(pixel, 1.0f);
		//console() << "Particle " << mState->iteration << " is  @ " << mParticles[mState->iteration].pos << "\n";
		mParticles[mState->iteration].color = mParticleColor;
		mParticles[mState->iteration].radius = randFloat(1.0f, 10.0f);
		++(*mState);
	}
}


void AttractorExplorer::mouseDown( MouseEvent event )
{
	moreParticles += 1000;
}

void AttractorExplorer::update()
{
	evaluateAttractor();
	// Copy particle data ont1o the GPU.
	// Map the GPU memory and write over it.
	void *gpuMem = mParticleVbo->mapReplace();
	memcpy( gpuMem, mParticles.data(), mState->iteration * sizeof(Particle) );
	mParticleVbo->unmap();
}

void AttractorExplorer::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatricesWindow( toPixels(getWindowSize()));
	gl::enableAlphaBlending();
	mTexture->bind();
	mParticleBatch->draw();
	mParams->draw();
}

#ifndef NDEBUG
CINDER_APP( AttractorExplorer, RendererGl, [] ( App::Settings *settings ) {
	settings->setWindowSize( 1280, 720 );
	settings->setMultiTouchEnabled( false );
	settings->setConsoleWindowEnabled();
})
#else
CINDER_APP(AttractorExplorer, RendererGl, [](App::Settings *settings) {
	settings->setWindowSize(1280, 720);
	settings->setMultiTouchEnabled( false );
})
#endif

