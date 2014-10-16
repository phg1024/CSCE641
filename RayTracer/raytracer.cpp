#include "raytracer.h"
#include "scene.h"

#include <cfloat>

#define USE_OPENMP 1

RayTracer::RayTracer():
    _scene(0),
    _canvas(0),
    _threaded(false),
    _isMSAAEnabled(true),
    _MSAASampleNumber(8),
    _bgColor(DblColor4(1, 1, 1, 1)),
    _directLightingFactor(1),
    _reflectionFactor(0.25),
    _refractionFactor(0.8),
    _maxIterations(16)
{
    _shiftVector[0] = DblVector2D(0, 0);
    _shiftVector[1] = DblVector2D(-0.25, -0.5);
    _shiftVector[2] = DblVector2D(-0.5, -0.25);
    _shiftVector[3] = DblVector2D(-0.25, -0.25);

    _shiftVector[4] = DblVector2D(-0.75, -0.75);
    _shiftVector[5] = DblVector2D(-0.5, -0.75);
    _shiftVector[6] = DblVector2D(-0.75, -0.5);
    _shiftVector[7] = DblVector2D(-0.5, -0.5);
}

void RayTracer::execute()
{
    if( _threaded )
    {
        rayTracing_Threaded();
    }
    else
    {
        rayTracing();
    }
}

void RayTracer::rayTracing()
{
    RGBAImage& image = (*_canvas->img);

#if !(USE_OPENMP)
    double accumulatedProgress = 0;
    double stepProgress = 1.0 / image.height();
#endif

#if USE_OPENMP
#pragma omp parallel for
#endif
    for(size_t i=0;i<image.height();i++)
    {
#if !(USE_OPENMP)
        accumulatedProgress += stepProgress;
        emit sig_progress(accumulatedProgress);
#endif

        float x;
        float y = (float)i / (float)image.height();
        for(size_t j=0;j<image.width();j++)
        {
            x = (float)j / (float)image.width();

            if( _isMSAAEnabled )
            {
                DblColor4 finalColor(0, 0, 0, 0);

                for(int k=0;k<_MSAASampleNumber;k++)
                {
                    DblPoint2D pos(x, y);
                    pos.x() = pos.x() + _shiftVector[k].x() / image.width();
                    pos.y() = pos.y() + _shiftVector[k].y() / image.height();
                    Ray r = _scene->cameraInfo().generateRay(pos);
                    r.order() = 0;
                    Hit h;

                    if( trace( r, h ) )
                    {
                        //cout << "(" << j << ", " << i << ")" << h.color() << endl;
                        finalColor = finalColor + h.color().clamp(0.0, 1.0);
                    }
                    else
                        finalColor = finalColor + _bgColor;
                }

                finalColor = finalColor / (double) _MSAASampleNumber;

                image.setPixel(j, i, finalColor.toRGBAPixel());
            }
            else
            {
                DblPoint2D pos(x, y);
                Ray r = _scene->cameraInfo().generateRay(pos);
                r.order() = 0;
                Hit h;

                if( trace( r, h ) )
                {
                    //cout << "(" << j << ", " << i << ")" << h.color() << endl;
                    image.setPixel(j, i, h.color().clamp(0.0, 1.0).toRGBAPixel());
                }
                else
                    image.setPixel(j, i, _bgColor.toRGBAPixel());
            }
        }
    }

    image.saveImage("result.png");

    _renderImage = image;
}

bool RayTracer::trace(const Ray &r, Hit &h)
{
    if( _scene->intersect(r, h) )
    {
        // termination for maximum iterations
        if( r.order() >= _maxIterations )
        {
            h.color() = evaluateLighting( r, h );
            return true;
        }

        bool compositionFlag[2] = {false};

        Hit reflectedHit;
        Hit refractedHit;

        // has reflected ray
        if( h.reflected() )
        {
            // trace reflected ray
            Ray reflectedRay = reflect( r, h );
            reflectedRay.order() = r.order() + 1;

            if( trace( reflectedRay, reflectedHit ) )
            {
                compositionFlag[0] = true;

                // inner reflection
                const double ZERO_THRESHD = 1e-9;
                if( abs(r.refractionRate() - h.refractionRate()) < ZERO_THRESHD
                && r.refractionRate() > 1 )
                {
                    // reduce the intensity of inner reflection
                    reflectedHit.color() = reflectedHit.color() * 0.25;
                }
            }
        }

        // has refracted ray
        if( h.refracted() )
        {
            // trace refracted ray
            Ray refractedRay = refract( r, h );
            refractedRay.order() = r.order() + 1;

            if( trace( refractedRay, refractedHit ) )
            {
                compositionFlag[1] = true;
                refractedHit.color() = DblColor4::blend(refractedHit.color(), h.color());
            }
        }

        // composite the colors
        DblColor4 directLighting = evaluateLighting( r, h );

        if( compositionFlag[1] )
        {
            h.color() = (directLighting * _directLightingFactor
                         + reflectedHit.color() * _reflectionFactor
                         + refractedHit.color() * _refractionFactor);
        }
        else if( compositionFlag[0] )
        {
            h.color() = (directLighting * _directLightingFactor
                         + reflectedHit.color() * _reflectionFactor);
        }
        else
            h.color() = directLighting;

        return true;
    }
    else
    {
        h.color() = _bgColor;
        return false;
    }
}

DblColor4 RayTracer::evaluateLighting(const Ray &r, const Hit &h)
{
    double constantAtt, linearAtt, quadraticAtt;
    constantAtt = 1;
    linearAtt = 0.025;
    quadraticAtt = 0.0005;

    // object property
    DblColor4 m_ambient(0.05, 0.05, 0.05, 1.0);
    DblColor4 m_diffuse = h.color();
    DblColor4 m_specular(1.0, 1.0, 1.0, 1.0);
    double m_shininess = 50.0;

    DblPoint3D hitPoint = r.origin() + r.dir() * h.t();
    DblVector3D N = normalize( h.normal() );

    DblColor4 accuVal(0, 0, 0, 1);

    // go through all light sources
    for(size_t i=0;i<_scene->lightSourcesNumber();i++)
    {
        const LightSource& l = _scene->lightSource(i);

        DblVector3D L = l._pos - hitPoint;

        double distLH = length( L );
        Ray rayLH;
        rayLH.dir() = normalize(L);
        rayLH.origin() = hitPoint;

        bool isTranslucent = false;
        DblColor4 lightColor = l._color;
        if( _scene->blockTest( rayLH, distLH, isTranslucent, lightColor ) )
        {
            if( !isTranslucent )
                continue;
            else
                ;
        }
        else
            lightColor = l._color;

        double d = length( L );
        double att = 1.0 / ( constantAtt + linearAtt * d + quadraticAtt * d * d);
        L = normalize( L );

        DblVector3D E = normalize( _scene->cameraInfo()._pos - hitPoint );

        double NdotL = dotProduct(N, L);
        if( NdotL < 0 ) NdotL = 0;

        DblColor4 Iamb = DblColor4(m_ambient.r() * h.color().r(),
                                   m_ambient.g() * h.color().g(),
                                   m_ambient.b() * h.color().b(),
                                   m_ambient.a() * h.color().a());

        DblVector3D R = normalize( 2.0 * NdotL * N - L );

        DblColor4 Idiff = m_diffuse * NdotL;

        double RdotE = dotProduct(R, E);
        if( RdotE < 0 ) RdotE = 0;
        DblColor4 Ispec = m_specular * pow( RdotE, m_shininess );

        double intensityFactor = 1.0;
        if( isTranslucent )
            intensityFactor = lightColor.a();
        accuVal = accuVal
                + (Idiff + Ispec + Iamb) * att * intensityFactor * powf(intensityFactor, 2.0)
                + (1.0 - powf(intensityFactor, 2.0) ) * lightColor;
    }

    accuVal = accuVal / (double) _scene->lightSourcesNumber();
    accuVal.a() = 1;

    return accuVal;
}

void RayTracer::rayTracing_Threaded()
{

}

void RayTracer::setSize(int w, int h)
{
    if( _canvas )
        delete _canvas;

    _canvas = new Canvas(w, h);
}

void RayTracer::bindScene(Scene *s)
{
    _scene = s;
}

Ray RayTracer::reflect(const Ray &r, const Hit &h)
{
    Ray fr;

    DblPoint3D hitPoint = r.origin() + r.dir() * h.t();

    DblVector3D N = normalize( h.normal() );
    DblVector3D L = r.dir() * -1;
    L = normalize( L );
    double NdotL = dotProduct(N, L);
    if( NdotL < 0 ) NdotL = 0;

    DblVector3D R = normalize( 2.0 * NdotL * N - L );

    fr.origin() = hitPoint;
    fr.dir() = R;
    fr.refractionRate() = r.refractionRate();

    return fr;
}

Ray RayTracer::refract(const Ray &r, const Hit &h)
{
    Ray fr;

    DblPoint3D hitPoint = r.origin() + r.dir() * h.t();

    DblVector3D N = normalize( h.normal() );
    DblVector3D L = r.dir();
    L = normalize( L );

    double LdotN = dotProduct(N, L);

    bool isEntering = (LdotN < 0);

    double c1, c2;

    if( isEntering )
    {        
        c2 = 1.0;
        c1 = h.refractionRate();
        //cout << "entering: " << c1 << ", " << c2 << endl;
        fr.refractionRate() = h.refractionRate();

        fr.origin() = hitPoint;
        fr.dir() = sqrt( c1 * c1 - c2 * c2 * ( 1.0 - LdotN * LdotN) ) / c1 * N * -1
                + c2 / c1 * (L - LdotN * N);
        fr.dir() = normalize( fr.dir() );

        //cout << "entering: " << length( fr.dir() - normalize( r.dir() ) ) << endl;
    }
    else
    {
        c1 = 1.0;
        c2 = h.refractionRate();
        //cout << "leaving: " << c1 << ", " << c2 << endl;
        fr.refractionRate() = 1.0;

        fr.origin() = hitPoint;
        double factor = c1 * c1 - c2 * c2 * ( 1.0 - LdotN * LdotN);
        if( factor <= 0 )
        {
            // should be total reflection, here simplify it with tangent leaving ray
            factor = 0;
        }
        fr.dir() = sqrt( factor ) / c1 * N
                + c2 / c1 * (L - LdotN * N);
        fr.dir() = normalize( fr.dir() );

        //cout << "leaving: " << length( fr.dir() - normalize( r.dir() ) ) << endl;
    }

    return fr;
}
