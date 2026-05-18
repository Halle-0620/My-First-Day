#include "shadertoywidget.h"

#include <QColor>
#include <QFile>
#include <QMouseEvent>
#include <QOpenGLTexture>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QDebug>

namespace {

const char *kVertexShaderSource = R"(#version 330 core
layout (location = 0) in vec2 aPosition;
out vec2 vUv;

void main()
{
    vUv = aPosition * 0.5 + 0.5;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)";

const char *kDriveHomeShaderSource = R"(#version 330 core
uniform vec3 iResolution;
uniform float iTime;
uniform vec4 iMouse;
in vec2 vUv;
out vec4 fragColor;

#define S(x, y, z) smoothstep(x, y, z)
#define B(a, b, edge, t) S(a-edge, a+edge, t)*S(b+edge, b-edge, t)
#define sat(x) clamp(x,0.,1.)

#define streetLightCol vec3(1., .7, .3)
#define headLightCol vec3(.8, .8, 1.)
#define tailLightCol vec3(1., .1, .1)

#define HIGH_QUALITY
#define CAM_SHAKE 1.
#define LANE_BIAS .5
#define RAIN

vec3 ro, rd;

float N(float t) {
    return fract(sin(t*10234.324)*123423.23512);
}
vec3 N31(float p) {
   vec3 p3 = fract(vec3(p) * vec3(.1031,.11369,.13787));
   p3 += dot(p3, p3.yzx + 19.19);
   return fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}
float N2(vec2 p)
{
    vec3 p3  = fract(vec3(p.xyx) * vec3(443.897, 441.423, 437.195));
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float DistLine(vec3 ro, vec3 rd, vec3 p) {
    return length(cross(p-ro, rd));
}

vec3 ClosestPoint(vec3 ro, vec3 rd, vec3 p) {
    return ro + max(0., dot(p-ro, rd))*rd;
}

float Remap(float a, float b, float c, float d, float t) {
    return ((t-a)/(b-a))*(d-c)+c;
}

float BokehMask(vec3 ro, vec3 rd, vec3 p, float size, float blur) {
    float d = DistLine(ro, rd, p);
    float m = S(size, size*(1.-blur), d);

    #ifdef HIGH_QUALITY
    m *= mix(.7, 1., S(.8*size, size, d));
    #endif

    return m;
}

float SawTooth(float t) {
    return cos(t+cos(t))+sin(2.*t)*.2+sin(4.*t)*.02;
}

float DeltaSawTooth(float t) {
    return 0.4*cos(2.*t)+0.08*cos(4.*t) - (1.-sin(t))*sin(t+cos(t));
}

vec2 GetDrops(vec2 uv, float seed, float m) {

    float t = iTime+m*30.;
    vec2 o = vec2(0.);

    uv.y += t*.05;

    uv *= vec2(10., 2.5)*2.;
    vec2 id = floor(uv);
    vec3 n = N31(id.x + (id.y+seed)*546.3524);
    vec2 bd = fract(uv);

    vec2 uv2 = bd;

    bd -= .5;

    bd.y*=4.;

    bd.x += (n.x-.5)*.6;

    t += n.z * 6.28;
    float slide = SawTooth(t);

    float ts = 1.5;
    vec2 trailPos = vec2(bd.x*ts, (fract(bd.y*ts*2.-t*2.)-.5)*.5);

    bd.y += slide*2.;

    #ifdef HIGH_QUALITY
    float dropShape = bd.x*bd.x;
    dropShape *= DeltaSawTooth(t);
    bd.y += dropShape;
    #endif

    float d = length(bd);

    float trailMask = S(-.2, .2, bd.y);
    trailMask *= bd.y;
    float td = length(trailPos*max(.5, trailMask));

    float mainDrop = S(.2, .1, d);
    float dropTrail = S(.1, .02, td);

    dropTrail *= trailMask;
    o = mix(bd*mainDrop, trailPos, dropTrail);

    return o;
}

void CameraSetup(vec2 uv, vec3 pos, vec3 lookat, float zoom, float m) {
    ro = pos;
    vec3 f = normalize(lookat-ro);
    vec3 r = cross(vec3(0., 1., 0.), f);
    vec3 u = cross(f, r);
    float t = iTime;

    vec2 offs = vec2(0.);
    vec2 dropUv = uv;

    #ifdef HIGH_QUALITY
    float x = (sin(t*.1)*.5+.5)*.5;
    x = -x*x;
    float s = sin(x);
    float c = cos(x);

    mat2 rot = mat2(c, -s, s, c);

    dropUv = uv*rot;
    dropUv.x += -sin(t*.1)*.5;
    #endif

    offs = GetDrops(dropUv, 1., m);
    offs += GetDrops(dropUv*1.4, 10., m);
    #ifdef HIGH_QUALITY
    offs += GetDrops(dropUv*2.4, 25., m);
    #endif

    float ripple = sin(t+uv.y*3.1415*30.+uv.x*124.)*.5+.5;
    ripple *= .005;
    offs += vec2(ripple*ripple, ripple);

    vec3 center = ro + f*zoom;
    vec3 i = center + (uv.x-offs.x)*r + (uv.y-offs.y)*u;

    rd = normalize(i-ro);
}

vec3 HeadLights(float i, float t) {
    float z = fract(-t*2.+i);
    vec3 p = vec3(-.3, .1, z*40.);
    float d = length(p-ro);

    float size = mix(.03, .05, S(.02, .07, z))*d;
    float m = 0.;
    float blur = .1;
    m += BokehMask(ro, rd, p-vec3(.08, 0., 0.), size, blur);
    m += BokehMask(ro, rd, p+vec3(.08, 0., 0.), size, blur);

    #ifdef HIGH_QUALITY
    m += BokehMask(ro, rd, p+vec3(.1, 0., 0.), size, blur);
    m += BokehMask(ro, rd, p-vec3(.1, 0., 0.), size, blur);
    #endif

    float distFade = max(.01, pow(1.-z, 9.));

    blur = .8;
    size *= 2.5;
    float r = 0.;
    r += BokehMask(ro, rd, p+vec3(-.09, -.2, 0.), size, blur);
    r += BokehMask(ro, rd, p+vec3(.09, -.2, 0.), size, blur);
    r *= distFade*distFade;

    return headLightCol*(m+r)*distFade;
}

vec3 TailLights(float i, float t) {
    t = t*1.5+i;

    float id = floor(t)+i;
    vec3 n = N31(id);

    float laneId = S(LANE_BIAS, LANE_BIAS+.01, n.y);

    float ft = fract(t);

    float z = 3.-ft*3.;

    laneId *= S(.2, 1.5, z);
    float lane = mix(.6, .3, laneId);
    vec3 p = vec3(lane, .1, z);
    float d = length(p-ro);

    float size = .05*d;
    float blur = .1;
    float m = BokehMask(ro, rd, p-vec3(.08, 0., 0.), size, blur) +
                BokehMask(ro, rd, p+vec3(.08, 0., 0.), size, blur);

    #ifdef HIGH_QUALITY
    float bs = n.z*3.;
    float brake = S(bs, bs+.01, z);
    brake *= S(bs+.01, bs, z-.5*n.y);

    m += (BokehMask(ro, rd, p+vec3(.1, 0., 0.), size, blur) +
        BokehMask(ro, rd, p-vec3(.1, 0., 0.), size, blur))*brake;
    #endif

    float refSize = size*2.5;
    m += BokehMask(ro, rd, p+vec3(-.09, -.2, 0.), refSize, .8);
    m += BokehMask(ro, rd, p+vec3(.09, -.2, 0.), refSize, .8);
    vec3 col = tailLightCol*m*ft;

    float b = BokehMask(ro, rd, p+vec3(.12, 0., 0.), size, blur);
    b += BokehMask(ro, rd, p+vec3(.12, -.2, 0.), refSize, .8)*.2;

    vec3 blinker = vec3(1., .7, .2);
    blinker *= S(1.5, 1.4, z)*S(.2, .3, z);
    blinker *= sat(sin(t*200.)*100.);
    blinker *= laneId;
    col += blinker*b;

    return col;
}

vec3 StreetLights(float i, float t) {
    float side = sign(rd.x);
    float offset = max(side, 0.)*(1./16.);
    float z = fract(i-t+offset);
    vec3 p = vec3(2.*side, 2., z*60.);
    float d = length(p-ro);
    float blur = .1;
    float distFade = Remap(1., .7, .1, 1.5, 1.-pow(1.-z,6.));
    distFade *= (1.-z);
    float m = BokehMask(ro, rd, p, .05*d, blur)*distFade;

    return m*streetLightCol;
}

vec3 EnvironmentLights(float i, float t) {
    float n = N(i+floor(t));

    float side = sign(rd.x);
    float offset = max(side, 0.)*(1./16.);
    float z = fract(i-t+offset+fract(n*234.));
    float n2 = fract(n*100.);
    vec3 p = vec3((3.+n)*side, n2*n2*n2*1., z*60.);
    float d = length(p-ro);
    float blur = .1;
    float distFade = Remap(1., .7, .1, 1.5, 1.-pow(1.-z,6.));
    float m = BokehMask(ro, rd, p, .05*d, blur);
    m *= distFade*distFade*.5;

    m *= 1.-pow(sin(z*6.28*20.*n)*.5+.5, 20.);
    vec3 randomCol = vec3(fract(n*-34.5), fract(n*4572.), fract(n*1264.));
    vec3 col = mix(tailLightCol, streetLightCol, fract(n*-65.42));
    col = mix(col, randomCol, n);
    return m*col*.2;
}

void mainImage(out vec4 outColor, in vec2 fragCoord)
{
    float t = iTime;
    vec3 col = vec3(0.);
    vec2 uv = fragCoord.xy / iResolution.xy;

    uv -= .5;
    uv.x *= iResolution.x/iResolution.y;

    vec2 mouse = iMouse.xy/iResolution.xy;

    vec3 pos = vec3(.3, .15, 0.);

    float bt = t * 5.;
    float h1 = N(floor(bt));
    float h2 = N(floor(bt+1.));
    float bumps = mix(h1, h2, fract(bt))*.1;
    bumps = bumps*bumps*bumps*CAM_SHAKE;

    pos.y += bumps;
    float lookatY = pos.y+bumps;
    vec3 lookat = vec3(0.3, lookatY, 1.);
    vec3 lookat2 = vec3(0., lookatY, .7);
    lookat = mix(lookat, lookat2, sin(t*.1)*.5+.5);

    uv.y += bumps*4.;
    CameraSetup(uv, pos, lookat, 2., mouse.x);

    t *= .03;
    t += mouse.x;

    float stp = 1./8.;

    for(float i=0.; i<1.; i+=stp) {
       col += StreetLights(i, t);
    }

    for(float i=0.; i<1.; i+=stp) {
        float n = N(i+floor(t));
        col += HeadLights(i+n*stp*.7, t);
    }

    #ifdef HIGH_QUALITY
    stp = 1./32.;
    #else
    stp = 1./16.;
    #endif

    for(float i=0.; i<1.; i+=stp) {
       col += EnvironmentLights(i, t);
    }

    col += TailLights(0., t);
    col += TailLights(.5, t);

    col += sat(rd.y)*vec3(.6, .5, .9);

    outColor = vec4(col, 1.0);
}

void main()
{
    mainImage(fragColor, gl_FragCoord.xy);
}
)";

const char *kTokyoRainShaderSource = R"(#version 330 core
uniform vec3 iResolution;
uniform float iTime;
uniform vec4 iMouse;
in vec2 vUv;
out vec4 fragColor;

#define BUMPMAP
#define MARCHSTEPS 128
#define MARCHSTEPSREFLECTION 48
#define LIGHTINTENSITY 5.

const vec3 backgroundColor = vec3(0.2,0.4,0.6) * 0.09;
#define time (iTime + 90.)

float hash( float n ) {
    return fract(sin(n)*687.3123);
}

float noise( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*157.0;
    return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
               mix( hash(n+157.0), hash(n+158.0),f.x),f.y);
}

const mat2 m2 = mat2( 0.80, -0.60, 0.60, 0.80 );

float fbm( vec2 p ) {
    float f = 0.0;
    f += 0.5000*noise( p ); p = m2*p*2.02;
    f += 0.2500*noise( p ); p = m2*p*2.03;
    f += 0.1250*noise( p ); p = m2*p*2.01;

    return f/0.9375;
}

float udRoundBox( vec3 p, vec3 b, float r ) {
  return length(max(abs(p)-b,0.0))-r;
}

float sdBox( in vec3 p, in vec3 b ) {
    vec3 d = abs(p) - b;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float sdSphere( in vec3 p, in float s ) {
    return length(p)-s;
}

float sdCylinder( in vec3 p, in vec2 h ) {
    vec2 d = abs(vec2(length(p.xz),p.y)) - h;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float opU( float d2, float d1 ) { return min( d1,d2); }
float opS( float d2, float d1 ) { return max(-d1,d2); }
float smin( float a, float b, float k ) { return -log(exp(-k*a)+exp(-k*b))/k; }

float mapCar(in vec3 p0){
    vec3 p=p0+vec3(0.0,1.24,0.0);
    float r=length(p.yz);
    float d= length(max(vec3(abs(p.x)-0.35,r-1.92,-p.y+1.4),0.0))-0.05;
    d=max(d,p.z-1.0);
    p=p0+vec3(0.0,-0.22,0.39);
    p.xz=abs(p.xz)-vec2(0.5300,0.9600);p.x=abs(p.x);
    r=length(p.yz);
    d=smin(d,length(max(vec3(p.x-0.08,r-0.25,-p.y-0.08),0.0))-0.04,8.0);
    d=max(d,-max(p.x-0.165,r-0.24));
    float d2=length(vec2(max(p.x-0.13,0.0),r-0.2))-0.02;
    d=min(d,d2);

    return d;
}

float dL;

float map( const in vec3 p ) {
    vec3 pd = p;
    float d;

    pd.x = abs( pd.x );
    pd.z *= -sign( p.x );

    float ch = hash( floor( (pd.z+18.*time)/40. ) );
    float lh = hash( floor( pd.z/13. ) );

    vec3 pdm = vec3( pd.x, pd.y, mod( pd.z, 10.) - 5. );
    dL = sdSphere( vec3(pdm.x-8.1,pdm.y-4.5,pdm.z), 0.1 );

    dL = opU( dL, sdBox( vec3(pdm.x-12., pdm.y-9.5-lh,  mod( pd.z, 91.) - 45.5 ), vec3(0.2,4.5, 0.2) ) );
    dL = opU( dL, sdBox( vec3(pdm.x-12., pdm.y-11.5+lh, mod( pd.z, 31.) - 15.5 ), vec3(0.22,5.5, 0.2) ) );
    dL = opU( dL, sdBox( vec3(pdm.x-12., pdm.y-8.5-lh,  mod( pd.z, 41.) - 20.5 ), vec3(0.24,3.5, 0.2) ) );

    if( lh > 0.5 ) {
        dL = opU( dL, sdBox( vec3(pdm.x-12.5,pdm.y-2.75-lh,  mod( pd.z, 13.) - 6.5 ), vec3(0.1,0.25, 3.2) ) );
    }

    vec3 pm = vec3( mod( pd.x + floor( pd.z * 4. )*0.25, 0.5 ) - 0.25, pd.y, mod( pd.z, 0.25 ) - 0.125 );
    d = udRoundBox( pm, vec3( 0.245,0.1, 0.12 ), 0.005 );

    d = opS( d, -(p.x+8.) );
    d = opU( d, pd.y );

    vec3 pdc = vec3( pd.x, pd.y, mod( pd.z+18.*time, 40.) - 20. );

    if( ch > 0.75 ) {
        pdc.x += (ch-0.75)*4.;
        dL = opU( dL, sdSphere( vec3( abs(pdc.x-5.)-1.05, pdc.y-0.55, pdc.z ),    0.025 ) );
        dL = opU( dL, sdSphere( vec3( abs(pdc.x-5.)-1.2,  pdc.y-0.65,  pdc.z+6.05 ), 0.025 ) );

        d = opU( d,  mapCar( (pdc-vec3(5.,-0.025,-2.3))*0.45 ) );
    }

    d = opU( d, 13.-pd.x );
    d = opU( d, sdCylinder( vec3(pdm.x-8.5, pdm.y, pdm.z), vec2(0.075,4.5)) );
    d = opU( d, dL );

    return d;
}

vec3 calcNormalSimple( in vec3 pos ) {
    const vec2 e = vec2(1.0,-1.0)*0.005;

    vec3 n = normalize( e.xyy*map( pos + e.xyy ) +
                        e.yyx*map( pos + e.yyx )   +
                        e.yxy*map( pos + e.yxy )   +
                        e.xxx*map( pos + e.xxx )   );
    return n;
}

vec3 calcNormal( in vec3 pos ) {
    vec3 n = calcNormalSimple( pos );
    if( pos.y > 0.12 ) return n;

    #ifdef BUMPMAP
    vec2 oc = floor( vec2(pos.x+floor( pos.z * 4. )*0.25, pos.z) * vec2( 2., 4. ) );

    if( abs(pos.x)<8. ) {
        oc = pos.xz;
    }

    vec3 p = pos * 250.;
    vec3 xn = 0.05*vec3(noise(p.xz)-0.5,0.,noise(p.zx)-0.5);
    xn += 0.1*vec3(fbm(oc.xy)-0.5,0.,fbm(oc.yx)-0.5);

    n = normalize( xn + n );
    #endif

    return n;
}

vec3 int1, int2, nor1;
vec4 lint1, lint2;

float intersect( in vec3 ro, in vec3 rd ) {
    const float precis = 0.001;
    float h = precis*2.0;
    float t = 0.;
    int1 = int2 = vec3( -500. );
    lint1 = lint2 = vec4( -500. );
    float mld = 100.;

    for( int i=0; i < MARCHSTEPS; i++ ) {
        h = map( ro+rd*t );
        if(dL < mld){
            mld=dL;
            lint1.xyz = ro+rd*t;
            lint1.w = abs(dL);
        }
        if( h < precis ) {
            int1.xyz = ro+rd*t;
            break;
        }
        t += max(h, precis*2.);
    }

    if( int1.z < -400. || t > 300.) {
        float d = -(ro.y + 0.1)/rd.y;
        if( d > 0. ) {
            int1.xyz = ro+rd*d;
        } else {
            return -1.;
        }
    }

    ro = ro + rd*t;
    nor1 = calcNormal(ro);
    ro += 0.01*nor1;
    rd = reflect( rd, nor1 );
    t = 0.0;
    h = precis*2.0;
    mld = 100.;

    for( int i=0; i < MARCHSTEPSREFLECTION; i++ ) {
        h = map( ro+rd*t );
        if(dL < mld){
            mld=dL;
            lint2.xyz = ro+rd*t;
            lint2.w = abs(dL);
        }
        if( h < precis ) {
            int2.xyz = ro+rd*t;
            return 1.;
        }
        t += max(h, precis*2.);
    }

    return 0.;
}

vec3 shade( in vec3 ro, in vec3 pos, in vec3 nor ) {
    vec3 col = vec3(0.5);

    if( abs(pos.x) > 15. || abs(pos.x) < 8. ) col = vec3( 0.02 );
    if( pos.y < 0.01 ) {
        if( abs( int1.x ) < 0.1 ) col = vec3( 0.9 );
        if( abs( abs( int1.x )-7.4 ) < 0.1 ) col = vec3( 0.9 );
    }

    float sh = clamp( dot( nor, normalize( vec3( -0.3, 0.3, -0.5 ) ) ), 0., 1.);
    col *= (sh * backgroundColor);

    if( abs( pos.x ) > 12.9 && pos.y > 9.) {
        float ha = hash(  133.1234*floor( pos.y / 3. ) + floor( (pos.z) / 3. ) );
        if( ha > 0.95) {
            col = ( (ha-0.95)*10.) * vec3( 1., 0.7, 0.4 );
        }
    }

    col = mix(  backgroundColor, col, exp( min(max(0.1*pos.y,0.25)-0.065*distance(pos, ro),0.) ) );

    return col;
}

vec3 getLightColor( in vec3 pos ) {
    vec3 lcol = vec3( 1., .7, .5 );

    vec3 pd = pos;
    pd.x = abs( pd.x );
    pd.z *= -sign( pos.x );

    float ch = hash( floor( (pd.z+18.*time)/40. ) );
    vec3 pdc = vec3( pd.x, pd.y, mod( pd.z+18.*time, 40.) - 20. );

    if( ch > 0.75 ) {
        pdc.x += (ch-0.75)*4.;
        if(  sdSphere( vec3( abs(pdc.x-5.)-1.05, pdc.y-0.55, pdc.z ), 0.25) < 2. ) {
            lcol = vec3( 1., 0.05, 0.01 );
        }
    }
    if( pd.y > 2. && abs(pd.x) > 10. && pd.y < 5. ) {
        float fl = floor( pd.z/13. );
        lcol = 0.4*lcol+0.5*vec3( hash( .1562+fl ), hash( .423134+fl ), 0. );
    }
    if(  abs(pd.x) > 10. && pd.y > 5. ) {
        float fl = floor( pd.z/2. );
        lcol = 0.5*lcol+0.5*vec3( hash( .1562+fl ),  hash( .923134+fl ), hash( .423134+fl ) );
    }

    return lcol;
}

float randomStart(vec2 co){return 0.8+0.2*hash(dot(co,vec2(123.42,117.853))*412.453);}

void mainImage( out vec4 outColor, in vec2 fragCoord ) {
    vec2 q = fragCoord.xy / iResolution.xy;
    vec2 p = -1.0 + 2.0*q;
    p.x *= iResolution.x / iResolution.y;

    if (q.y < .12 || q.y >= .88) {
        outColor=vec4(0.,0.,0.,1.);
        return;
    } else {
        float z = time;
        float x = -10.9+1.*sin(time*0.2);
        vec3 ro = vec3(x,  1.3+.3*cos(time*0.26), z-1.);
        vec3 ta = vec3(-8.,1.3+.4*cos(time*0.26), z+4.+cos(time*0.04));

        vec3 ww = normalize( ta - ro );
        vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
        vec3 vv = normalize( cross(uu,ww));
        vec3 rd = normalize( -p.x*uu + p.y*vv + 2.2*ww );

        vec3 col = backgroundColor;

        float ints = intersect(ro+randomStart(p)*rd ,rd );
        if(  ints > -0.5 ) {
            float r = 0.09;
            if( int1.y > 0.129 ) r = 0.025 * hash(  133.1234*floor( int1.y / 3. ) + floor( int1.z / 3. ) );
            if( abs(int1.x) < 8. ) {
                if( int1.y < 0.01 ) {
                    r = 0.007*fbm(int1.xz);
                } else {
                    r = 0.02;
                }
            }
            if( abs( int1.x ) < 0.1 ) r *= 4.;
            if( abs( abs( int1.x )-7.4 ) < 0.1 ) r *= 4.;

            r *= 2.;

            col = shade( ro, int1.xyz, nor1 );

            if( ints > 0.5 ) {
                col += r * shade( int1.xyz, int2.xyz, calcNormalSimple(int2.xyz) );
            }
            if( lint2.w > 0. ) {
                col += (r*LIGHTINTENSITY*exp(-lint2.w*7.0)) * getLightColor(lint2.xyz);
            }
        }

        vec2 st = 256. * ( p* vec2(.5, .01)+vec2(time*.13-q.y*.6, time*.13) );
        float f = noise( st ) * noise( st*0.773) * 1.55;
        f = 0.25+ clamp(pow(abs(f), 13.0) * 13.0, 0.0, q.y*.14);

        if( lint1.w > 0. ) {
            col += (f*LIGHTINTENSITY*exp(-lint1.w*7.0)) * getLightColor(lint1.xyz);
        }

        col += 0.25*f*(0.2+backgroundColor);

        col = pow( clamp(col,0.0,1.0), vec3(0.4545) );
        col *= 1.2*vec3(1.,0.99,0.95);
        col = clamp(1.06*col-0.03, 0., 1.);
        q.y = (q.y-.12)*(1./0.76);
        col *= 0.5 + 0.5*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );

        outColor = vec4( col, 1.0 );
    }
}

void main()
{
    mainImage(fragColor, gl_FragCoord.xy);
}
)";

const char *kDreamSleepBlurShaderSource = R"(#version 330 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;
in vec2 vUv;
out vec4 fragColor;

const int SAMPLES = 24;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(41.0, 289.0))) * 45758.5453);
}

vec4 sampleScene(vec2 uv) {
    return texture(iChannel0, clamp(uv, vec2(0.001), vec2(0.999)));
}

void mainImage(out vec4 outColor, in vec2 fragCoord) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 sceneUv = vec2(uv.x, 1.0 - uv.y);

    float progress = smoothstep(0.0, 1.0, min(iTime / 4.6, 1.0));
    float decay = mix(0.985, 0.94, progress);
    float density = mix(0.08, 0.62, progress);
    float weight = mix(0.015, 0.10, progress);

    vec2 focus = vec2(0.52, 0.44) + 0.024 * vec2(sin(iTime * 0.42), cos(iTime * 0.31));
    vec2 radial = uv - focus;
    vec2 delta = radial * density / float(SAMPLES);
    vec2 sampleUv = sceneUv + vec2(delta.x, -delta.y) * (hash(uv + fract(iTime)) * 2.0 - 1.0);

    vec4 base = sampleScene(sceneUv);
    vec4 col = base * mix(1.0, 0.28, progress);
    float localWeight = weight;

    for (int i = 0; i < SAMPLES; ++i) {
        sampleUv -= vec2(delta.x, -delta.y);
        col += sampleScene(sampleUv) * localWeight;
        localWeight *= decay;
    }

    float vignette = clamp(1.0 - dot(radial, radial) * mix(0.18, 0.82, progress), 0.0, 1.0);
    vec3 color = mix(base.rgb, col.rgb, 0.2 + 0.8 * progress);
    color *= mix(1.0, 0.48, progress);
    color *= vignette;
    color += vec3(0.012, 0.015, 0.024) * progress;

    outColor = vec4(sqrt(clamp(color, 0.0, 1.0)), 1.0);
}

void main() {
    mainImage(fragColor, gl_FragCoord.xy);
}
)";

const char *kDreamFallShaderSource = R"(#version 330 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;
in vec2 vUv;
out vec4 fragColor;

const int SAMPLES = 32;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(17.1, 91.7))) * 43758.5453);
}

vec4 sampleScene(vec2 uv) {
    return texture(iChannel0, clamp(uv, vec2(0.001), vec2(0.999)));
}

void mainImage(out vec4 outColor, in vec2 fragCoord) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 sceneUv = vec2(uv.x, 1.0 - uv.y);
    float progress = clamp(iTime / 5.0, 0.0, 1.0);

    vec2 focus = vec2(0.5, 0.52) + 0.08 * vec2(sin(iTime * 0.55), cos(iTime * 0.73));
    vec2 warped = uv;
    warped += 0.018 * progress * vec2(
        sin(uv.y * 18.0 + iTime * 2.6),
        cos(uv.x * 16.0 - iTime * 2.1)
    );
    float shard = sin((uv.x + uv.y + iTime * 0.8) * 28.0) * cos((uv.x - uv.y - iTime * 0.6) * 23.0);
    warped += progress * 0.025 * vec2(shard, -shard);

    vec2 radial = warped - focus;
    vec2 delta = radial * mix(0.28, 0.95, progress) / float(SAMPLES);
    vec2 sampleUv = vec2(warped.x, 1.0 - warped.y);

    vec4 col = sampleScene(sampleUv) * 0.18;
    float weight = 0.14;
    float decay = 0.955;

    for (int i = 0; i < SAMPLES; ++i) {
        sampleUv -= vec2(delta.x, -delta.y);
        col += sampleScene(sampleUv) * weight;
        weight *= decay;
    }

    float fracture = smoothstep(0.42, 0.0, abs(shard)) * progress;
    vec3 base = col.rgb;
    vec3 chroma;
    chroma.r = sampleScene(vec2(warped.x + 0.012 * fracture, 1.0 - warped.y)).r;
    chroma.g = sampleScene(vec2(warped.x, 1.0 - warped.y)).g;
    chroma.b = sampleScene(vec2(warped.x - 0.012 * fracture, 1.0 - warped.y)).b;
    vec3 color = mix(base, chroma, 0.42);

    float tunnel = clamp(1.0 - dot(radial, radial) * 1.35, 0.0, 1.0);
    float pulse = 0.82 + 0.18 * sin(iTime * 3.2 + uv.y * 12.0);
    color *= tunnel * pulse;
    color *= mix(0.86, 0.38, progress);
    color += vec3(0.02, 0.025, 0.035) * fracture;
    color = pow(clamp(color, 0.0, 1.0), vec3(0.9));

    outColor = vec4(color, 1.0);
}

void main() {
    mainImage(fragColor, gl_FragCoord.xy);
}
)";

const char *kDreamFutureShaderSource = R"(#version 330 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;
in vec2 vUv;
out vec4 fragColor;

const float FLIGHT_SPEED = 8.0;
const float DRAW_DISTANCE = 60.0;
const float FADEOUT_DISTANCE = 10.0;
const float FIELD_OF_VIEW = 1.05;

const float STAR_SIZE = 0.6;
const float STAR_CORE_SIZE = 0.14;

const float CLUSTER_SCALE = 0.02;
const float STAR_THRESHOLD = 0.775;

const float BLACK_HOLE_CORE_RADIUS = 0.2;
const float BLACK_HOLE_THRESHOLD = 0.9995;
const float BLACK_HOLE_DISTORTION = 0.03;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898, 78.233))) * 43758.5453);
}

float safeDiv(float a, float b) {
    return a / (abs(b) < 0.001 ? (b < 0.0 ? -1.0 : 1.0) : b);
}

vec3 getRayDirection(vec2 fragCoord, vec3 cameraDirection) {
    vec2 uv = fragCoord.xy / iResolution.xy;

    const float screenWidth = 1.0;
    float originToScreen = screenWidth / 2.0 / tan(FIELD_OF_VIEW / 2.0);

    vec3 screenCenter = originToScreen * cameraDirection;
    vec3 baseX = normalize(cross(screenCenter, vec3(0, -1.0, 0)));
    vec3 baseY = normalize(cross(screenCenter, baseX));

    return normalize(screenCenter + (uv.x - 0.5) * baseX + (uv.y - 0.5) * iResolution.y / iResolution.x * baseY);
}

float getDistance(ivec3 chunkPath, vec3 localStart, vec3 localPosition) {
    return length(vec3(chunkPath) + localPosition - localStart);
}

void move(inout vec3 localPosition, vec3 rayDirection, vec3 directionBound) {
    vec3 directionSign = sign(rayDirection);
    vec3 amountVector = (directionBound - directionSign * localPosition) / abs(rayDirection);

    float amount = min(amountVector.x, min(amountVector.y, amountVector.z));
    localPosition += amount * rayDirection;
}

void moveInsideBox(inout vec3 localPosition, inout ivec3 chunk, vec3 directionSign, vec3 direcctionBound) {
    const float eps = 0.0000001;
    if (localPosition.x * directionSign.x >= direcctionBound.x - eps) {
        localPosition.x -= directionSign.x;
        chunk.x += int(directionSign.x);
    } else if (localPosition.y * directionSign.y >= direcctionBound.y - eps) {
        localPosition.y -= directionSign.y;
        chunk.y += int(directionSign.y);
    } else if (localPosition.z * directionSign.z >= direcctionBound.z - eps) {
        localPosition.z -= directionSign.z;
        chunk.z += int(directionSign.z);
    }
}

bool hasStar(ivec3 chunk) {
    return texture(iChannel0, mod(CLUSTER_SCALE * (vec2(chunk.xy) + vec2(chunk.zx)) + vec2(0.724, 0.111), 1.0)).r > STAR_THRESHOLD
        && texture(iChannel0, mod(CLUSTER_SCALE * (vec2(chunk.xz) + vec2(chunk.zy)) + vec2(0.333, 0.777), 1.0)).r > STAR_THRESHOLD;
}

bool hasBlackHole(ivec3 chunk) {
    return rand(0.0001 * vec2(chunk.xy) + 0.002 * vec2(chunk.yz)) > BLACK_HOLE_THRESHOLD;
}

vec3 getStarToRayVector(vec3 rayBase, vec3 rayDirection, vec3 starPosition) {
    float r = (dot(rayDirection, starPosition) - dot(rayDirection, rayBase)) / dot(rayDirection, rayDirection);
    vec3 pointOnRay = rayBase + r * rayDirection;
    return pointOnRay - starPosition;
}

vec3 getStarPosition(ivec3 chunk, float starSize) {
    float x = rand(vec2(safeDiv(float(chunk.x), float(chunk.y)) + 0.24, safeDiv(float(chunk.y), float(chunk.z)) + 0.66));
    float y = rand(vec2(safeDiv(float(chunk.x), float(chunk.z)) + 0.73, safeDiv(float(chunk.z), float(chunk.y)) + 0.45));
    float z = rand(vec2(safeDiv(float(chunk.y), float(chunk.x)) + 0.12, safeDiv(float(chunk.y), float(chunk.z)) + 0.76));
    vec3 position = abs(vec3(x, y, z));

    return starSize * vec3(1.0) + (1.0 - 2.0 * starSize) * position;
}

vec4 getNebulaColor(vec3 globalPosition, vec3 rayDirection) {
    vec3 color = vec3(0.0);
    float spaceLeft = 1.0;

    const float layerDistance = 10.0;
    float rayLayerStep = rayDirection.z / layerDistance;

    const int steps = 4;
    for (int i = 0; i <= steps; i++) {
        vec3 noiseeval = globalPosition + rayDirection * ((1.0 - fract(globalPosition.z / layerDistance) + float(i)) * layerDistance / max(rayDirection.z, 0.001));
        noiseeval.xy += noiseeval.z;

        float value = 0.06 * texture(iChannel0, fract(noiseeval.xy / 60.0)).r;

        if (i == 0) {
            value *= 1.0 - fract(globalPosition.z / layerDistance);
        } else if (i == steps) {
            value *= fract(globalPosition.z / layerDistance);
        }

        float hue = mod(noiseeval.z / layerDistance / 34.444, 1.0);

        color += spaceLeft * hsv2rgb(vec3(hue, 0.72, value * 0.75));
        spaceLeft = max(0.0, spaceLeft - value * 2.0);
    }
    return vec4(color, 1.0);
}

vec4 getStarGlowColor(float starDistance, float angle, float hue) {
    float progress = 1.0 - starDistance;
    vec3 glow = hsv2rgb(vec3(hue, 0.22, 1.0));
    return vec4(glow, 0.32 * pow(progress, 2.0) * mix(pow(abs(sin(angle * 2.5)), 8.0), 1.0, progress));
}

float atan2(vec2 value) {
    if (value.x > 0.0) {
        return atan(value.y / value.x);
    } else if (value.x == 0.0) {
        return 3.14592 * 0.5 * sign(value.y);
    } else if (value.y >= 0.0) {
        return atan(value.y / value.x) + 3.141592;
    } else {
        return atan(value.y / value.x) - 3.141592;
    }
}

vec3 getStarColor(vec3 starSurfaceLocation, float seed, float viewDistance) {
    const float DISTANCE_FAR = 20.0;
    const float DISTANCE_NEAR = 15.0;

    if (viewDistance > DISTANCE_FAR) {
        return vec3(1.0);
    }

    float fadeToWhite = max(0.0, (viewDistance - DISTANCE_NEAR) / (DISTANCE_FAR - DISTANCE_NEAR));
    vec3 coordinate = vec3(acos(starSurfaceLocation.y), atan2(starSurfaceLocation.xz), seed);
    float progress = pow(texture(iChannel0, fract(0.3 * coordinate.xy + seed * vec2(1.1))).r, 4.0);

    return mix(mix(vec3(1.0, 0.98, 0.9), vec3(1.0, 0.627, 0.01), progress), vec3(1.0), fadeToWhite);
}

vec4 blendColors(vec4 front, vec4 back) {
    return vec4(mix(back.rgb, front.rgb, front.a / max(front.a + back.a, 0.0001)), front.a + back.a - front.a * back.a);
}

void mainImage(out vec4 outColor, in vec2 fragCoord) {
    vec3 movementDirection = normalize(vec3(0.01, 0.0, 1.0));
    vec3 rayDirection = getRayDirection(fragCoord, movementDirection);
    vec3 directionSign = sign(rayDirection);
    vec3 directionBound = vec3(0.5) + 0.5 * directionSign;

    vec3 globalPosition = vec3(3.14159, 3.14159, 0.0) + (iTime + 1000.0) * FLIGHT_SPEED * movementDirection;
    ivec3 chunk = ivec3(globalPosition);
    vec3 localPosition = mod(globalPosition, 1.0);
    moveInsideBox(localPosition, chunk, directionSign, directionBound);

    ivec3 startChunk = chunk;
    vec3 localStart = localPosition;

    outColor = vec4(0.0);

    for (int i = 0; i < 200; i++) {
        move(localPosition, rayDirection, directionBound);
        moveInsideBox(localPosition, chunk, directionSign, directionBound);

        if (hasStar(chunk)) {
            vec3 starPosition = getStarPosition(chunk, 0.5 * STAR_SIZE);
            float currentDistance = getDistance(chunk - startChunk, localStart, starPosition);

            vec3 starToRayVector = getStarToRayVector(localPosition, rayDirection, starPosition);
            float distanceToStar = length(starToRayVector) * 2.0;

            if (distanceToStar < STAR_SIZE) {
                float starMaxBrightness = clamp((DRAW_DISTANCE - currentDistance) / FADEOUT_DISTANCE, 0.001, 1.0);
                float starColorSeed = (float(chunk.x) + 13.0 * float(chunk.y) + 7.0 * float(chunk.z)) * 0.00453;
                if (distanceToStar < STAR_SIZE * STAR_CORE_SIZE) {
                    vec3 starSurfaceVector = normalize(starToRayVector + rayDirection * sqrt(pow(STAR_CORE_SIZE * STAR_SIZE, 2.0) - pow(distanceToStar, 2.0)));
                    outColor = blendColors(outColor, vec4(getStarColor(starSurfaceVector, starColorSeed, currentDistance), starMaxBrightness));
                    break;
                } else {
                    float localStarDistance = ((distanceToStar / STAR_SIZE) - STAR_CORE_SIZE) / (1.0 - STAR_CORE_SIZE);
                    vec4 glowColor = getStarGlowColor(localStarDistance, atan2(starToRayVector.xy), starColorSeed);
                    glowColor.a *= starMaxBrightness;
                    outColor = blendColors(outColor, glowColor);
                }
            }
        } else if (hasBlackHole(chunk)) {
            const vec3 blackHolePosition = vec3(0.5);
            float currentDistance = getDistance(chunk - startChunk, localStart, blackHolePosition);
            float fadeout = min(1.0, (DRAW_DISTANCE - currentDistance) / FADEOUT_DISTANCE);

            vec3 coreToRayVector = getStarToRayVector(localPosition, rayDirection, blackHolePosition);
            float distanceToCore = length(coreToRayVector);
            if (distanceToCore < BLACK_HOLE_CORE_RADIUS * 0.5) {
                outColor = blendColors(outColor, vec4(vec3(0.0), fadeout));
                break;
            } else if (distanceToCore < 0.5) {
                rayDirection = normalize(rayDirection - fadeout * (BLACK_HOLE_DISTORTION / distanceToCore - BLACK_HOLE_DISTORTION / 0.5) * coreToRayVector / distanceToCore);
            }
        }

        if (length(vec3(chunk - startChunk)) > DRAW_DISTANCE) {
            break;
        }
    }

    if (outColor.a < 1.0) {
        outColor = blendColors(outColor, getNebulaColor(globalPosition, rayDirection));
    }

    outColor.rgb = pow(clamp(outColor.rgb, 0.0, 1.0), vec3(0.92));
    outColor.rgb *= vec3(0.78, 0.82, 0.94);
    outColor.rgb *= 0.68;
    outColor.a = 1.0;
}

void main() {
    mainImage(fragColor, gl_FragCoord.xy);
}
)";

} // namespace

ShaderToyWidget::ShaderToyWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
{
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setMouseTracking(true);
}

ShaderToyWidget::~ShaderToyWidget()
{
    makeCurrent();
    destroyProgram();
    destroyTextures();
    m_vertexArray.destroy();
    m_vertexBuffer.destroy();
    doneCurrent();
}

void ShaderToyWidget::setShaderEffect(ShaderEffect effect)
{
    if (m_shaderEffect == effect) {
        return;
    }

    m_shaderEffect = effect;
    m_compiledProgramKey.clear();
    update();
}

void ShaderToyWidget::setDreamShaderEnabled(bool enabled)
{
    if (m_dreamShaderEnabled == enabled) {
        return;
    }

    m_dreamShaderEnabled = enabled;
    m_compiledProgramKey.clear();
    if (!m_dreamShaderEnabled) {
        destroyProgram();
    }
    update();
}

void ShaderToyWidget::setExternalFragmentShaderFile(const QString &path)
{
    const QString normalizedPath = path.trimmed();
    if (m_externalFragmentShaderFile == normalizedPath) {
        return;
    }

    m_externalFragmentShaderFile = normalizedPath;
    m_compiledProgramKey.clear();
    destroyProgram();
    update();
}

void ShaderToyWidget::setExternalChannel0UsesNoiseTexture(bool useNoiseTexture)
{
    if (m_externalChannel0UsesNoiseTexture == useNoiseTexture) {
        return;
    }

    m_externalChannel0UsesNoiseTexture = useNoiseTexture;
    update();
}

void ShaderToyWidget::setSourcePixmap(const QPixmap &pixmap)
{
    const QImage newImage = pixmap.isNull()
        ? QImage()
        : pixmap.toImage().convertToFormat(QImage::Format_RGBA8888);
    if (m_sourceImage.cacheKey() == newImage.cacheKey()) {
        return;
    }

    m_sourceImage = newImage;
    m_sourceTextureDirty = true;
    update();
}

ShaderEffect ShaderToyWidget::shaderEffect() const
{
    return m_shaderEffect;
}

bool ShaderToyWidget::isDreamShaderEnabled() const
{
    return m_dreamShaderEnabled;
}

QString ShaderToyWidget::externalFragmentShaderFile() const
{
    return m_externalFragmentShaderFile;
}

bool ShaderToyWidget::externalChannel0UsesNoiseTexture() const
{
    return m_externalChannel0UsesNoiseTexture;
}

bool ShaderToyWidget::hasActiveShader() const
{
    return (m_dreamShaderEnabled && !m_externalFragmentShaderFile.isEmpty())
        || m_shaderEffect != ShaderEffect::None;
}

void ShaderToyWidget::restartAnimation()
{
    m_elapsedTimer.restart();
    update();
}

void ShaderToyWidget::initializeGL()
{
    initializeOpenGLFunctions();

    m_vertexArray.create();
    m_vertexArray.bind();

    m_vertexBuffer.create();
    m_vertexBuffer.bind();

    const GLfloat vertices[] = {
        -1.f, -1.f,
         1.f, -1.f,
        -1.f,  1.f,
         1.f,  1.f
    };
    m_vertexBuffer.allocate(vertices, static_cast<int>(sizeof(vertices)));

    m_elapsedTimer.start();
    m_sourceTextureDirty = true;
    m_noiseTextureDirty = true;
}

void ShaderToyWidget::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    if (m_mousePosition.isNull()) {
        m_mousePosition = QPointF(width() * 0.5, height() * 0.5);
        m_mousePressPosition = m_mousePosition;
    }
}

void ShaderToyWidget::paintGL()
{
    const qreal dpr = devicePixelRatioF();
    const int framebufferWidth = qRound(width() * dpr);
    const int framebufferHeight = qRound(height() * dpr);

    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!hasActiveShader()) {
        return;
    }

    if (!ensureProgram()) {
        return;
    }

    updateSourceTexture();
    updateNoiseTexture();

    m_program->bind();
    m_vertexArray.bind();
    m_vertexBuffer.bind();

    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2, 0);
    m_program->setUniformValue("iResolution", QVector3D(framebufferWidth, framebufferHeight, 1.0f));
    m_program->setUniformValue("iTime", float(m_elapsedTimer.elapsed()) / 1000.0f);
    m_program->setUniformValue("iMouse", mouseUniformValue());
    const bool useExternalShader = m_dreamShaderEnabled && !m_externalFragmentShaderFile.isEmpty();
    const bool useExternalSourceTexture = useExternalShader && !m_externalChannel0UsesNoiseTexture;
    const bool useExternalNoiseTexture = useExternalShader && m_externalChannel0UsesNoiseTexture;
    if ((useExternalSourceTexture || effectUsesSourceTexture(m_shaderEffect)) && m_sourceTexture) {
        m_program->setUniformValue("iChannel0", 0);
        glActiveTexture(GL_TEXTURE0);
        m_sourceTexture->bind();
    } else if ((useExternalNoiseTexture || effectUsesNoiseTexture(m_shaderEffect)) && m_noiseTexture) {
        m_program->setUniformValue("iChannel0", 0);
        glActiveTexture(GL_TEXTURE0);
        m_noiseTexture->bind();
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if ((useExternalSourceTexture || effectUsesSourceTexture(m_shaderEffect)) && m_sourceTexture) {
        m_sourceTexture->release();
    } else if ((useExternalNoiseTexture || effectUsesNoiseTexture(m_shaderEffect)) && m_noiseTexture) {
        m_noiseTexture->release();
    }

    m_program->disableAttributeArray(0);
    m_program->release();

    update();
}

void ShaderToyWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePosition = event->position();
    update();
    QOpenGLWidget::mouseMoveEvent(event);
}

void ShaderToyWidget::mousePressEvent(QMouseEvent *event)
{
    m_mousePressed = true;
    m_mousePosition = event->position();
    m_mousePressPosition = event->position();
    update();
    QOpenGLWidget::mousePressEvent(event);
}

void ShaderToyWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_mousePressed = false;
    m_mousePosition = event->position();
    update();
    QOpenGLWidget::mouseReleaseEvent(event);
}

bool ShaderToyWidget::ensureProgram()
{
    const QString programKey = currentProgramKey();
    if (programKey.isEmpty()) {
        return false;
    }

    if (m_program && m_compiledProgramKey == programKey) {
        return true;
    }

    destroyProgram();

    auto *program = new QOpenGLShaderProgram(this);
    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShaderSource)) {
        reportShaderCompileFailure(QStringLiteral("Vertex shader compile failed: %1").arg(program->log()));
        delete program;
        return false;
    }

    QString sourceError;
    const QString fragmentSource = currentFragmentSource(&sourceError);
    if (fragmentSource.isEmpty()) {
        reportShaderCompileFailure(sourceError.isEmpty()
            ? QStringLiteral("Fragment shader source is empty.")
            : sourceError);
        delete program;
        return false;
    }

    if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource)) {
        reportShaderCompileFailure(QStringLiteral("Fragment shader compile failed for %1: %2")
            .arg(programKey, program->log()));
        delete program;
        return false;
    }

    if (!program->link()) {
        reportShaderCompileFailure(QStringLiteral("Shader program link failed for %1: %2")
            .arg(programKey, program->log()));
        delete program;
        return false;
    }

    m_program = program;
    m_compiledProgramKey = programKey;
    m_lastFailedProgramKey.clear();
    return true;
}

QString ShaderToyWidget::currentProgramKey() const
{
    if (m_dreamShaderEnabled && !m_externalFragmentShaderFile.isEmpty()) {
        return QStringLiteral("external:%1").arg(m_externalFragmentShaderFile);
    }

    if (m_shaderEffect == ShaderEffect::None) {
        return {};
    }

    return QStringLiteral("effect:%1").arg(int(m_shaderEffect));
}

QString ShaderToyWidget::currentFragmentSource(QString *errorMessage) const
{
    if (m_dreamShaderEnabled && !m_externalFragmentShaderFile.isEmpty()) {
        QFile shaderFile(m_externalFragmentShaderFile);
        if (!shaderFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Failed to open shader file: %1").arg(m_externalFragmentShaderFile);
            }
            return {};
        }

        return buildExternalFragmentSource(QString::fromUtf8(shaderFile.readAll()));
    }

    const QString source = fragmentSourceForEffect(m_shaderEffect);
    if (source.isEmpty() && errorMessage) {
        *errorMessage = QStringLiteral("No built-in shader source for effect %1.").arg(int(m_shaderEffect));
    }
    return source;
}

bool ShaderToyWidget::effectUsesSourceTexture(ShaderEffect effect) const
{
    return effect == ShaderEffect::DreamSleepBlur
        || effect == ShaderEffect::DreamFall;
}

bool ShaderToyWidget::effectUsesNoiseTexture(ShaderEffect effect) const
{
    return effect == ShaderEffect::DreamFuture;
}

QString ShaderToyWidget::fragmentSourceForEffect(ShaderEffect effect) const
{
    switch (effect) {
    case ShaderEffect::DriveHome:
        return QString::fromUtf8(kDriveHomeShaderSource);
    case ShaderEffect::TokyoRain:
        return QString::fromUtf8(kTokyoRainShaderSource);
    case ShaderEffect::DreamSleepBlur:
        return QString::fromUtf8(kDreamSleepBlurShaderSource);
    case ShaderEffect::DreamFall:
        return QString::fromUtf8(kDreamFallShaderSource);
    case ShaderEffect::DreamFuture:
        return QString::fromUtf8(kDreamFutureShaderSource);
    case ShaderEffect::None:
        break;
    }

    return {};
}

QString ShaderToyWidget::buildExternalFragmentSource(const QString &source) const
{
    QString raw = source;
    raw.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    raw = raw.trimmed();
    if (raw.isEmpty()) {
        return {};
    }

    QString versionLine = QStringLiteral("#version 330 core");
    const QRegularExpression versionRegex(QStringLiteral("^\\s*#version[^\\n]*"), QRegularExpression::MultilineOption);
    const QRegularExpressionMatch versionMatch = versionRegex.match(raw);
    if (versionMatch.hasMatch()) {
        versionLine = versionMatch.captured(0).trimmed();
        raw.remove(versionMatch.capturedStart(0), versionMatch.capturedLength(0));
        raw = raw.trimmed();
    }

    QStringList preamble;
    preamble << versionLine;
    preamble << QStringLiteral("uniform vec3 iResolution;");
    preamble << QStringLiteral("uniform float iTime;");
    preamble << QStringLiteral("uniform vec4 iMouse;");
    preamble << QStringLiteral("uniform sampler2D iChannel0;");
    preamble << QStringLiteral("in vec2 vUv;");
    preamble << QStringLiteral("out vec4 fragColor;");

    QString wrapped = preamble.join(QStringLiteral("\n"));
    wrapped += QStringLiteral("\n\n");
    wrapped += raw;
    wrapped += QStringLiteral("\n");

    if (!raw.contains(QRegularExpression(QStringLiteral("\\bvoid\\s+main\\s*\\(")))) {
        wrapped += QStringLiteral(
            "\nvoid main()\n"
            "{\n"
            "    mainImage(fragColor, gl_FragCoord.xy);\n"
            "}\n");
    }

    return wrapped;
}

void ShaderToyWidget::destroyProgram()
{
    delete m_program;
    m_program = nullptr;
    m_compiledProgramKey.clear();
}

void ShaderToyWidget::destroyTextures()
{
    delete m_sourceTexture;
    m_sourceTexture = nullptr;
    delete m_noiseTexture;
    m_noiseTexture = nullptr;
    m_sourceTextureDirty = true;
    m_noiseTextureDirty = true;
}

void ShaderToyWidget::updateSourceTexture()
{
    if (!m_sourceTextureDirty) {
        return;
    }

    delete m_sourceTexture;
    m_sourceTexture = nullptr;

    QImage image = m_sourceImage;
    if (image.isNull()) {
        image = QImage(1, 1, QImage::Format_RGBA8888);
        image.fill(QColor(12, 14, 22));
    }

    image = image.convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);
    m_sourceTexture = new QOpenGLTexture(image);
    m_sourceTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_sourceTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_sourceTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_sourceTextureDirty = false;
}

void ShaderToyWidget::updateNoiseTexture()
{
    if (!m_noiseTextureDirty) {
        return;
    }

    delete m_noiseTexture;
    m_noiseTexture = nullptr;

    QImage noiseImage(256, 256, QImage::Format_RGBA8888);
    for (int y = 0; y < noiseImage.height(); ++y) {
        uchar *line = noiseImage.scanLine(y);
        for (int x = 0; x < noiseImage.width(); ++x) {
            const uchar value = static_cast<uchar>(QRandomGenerator::global()->bounded(256));
            const int index = x * 4;
            line[index + 0] = value;
            line[index + 1] = value;
            line[index + 2] = value;
            line[index + 3] = 255;
        }
    }

    noiseImage = noiseImage.flipped(Qt::Vertical);
    m_noiseTexture = new QOpenGLTexture(noiseImage);
    m_noiseTexture->setWrapMode(QOpenGLTexture::Repeat);
    m_noiseTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_noiseTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    m_noiseTextureDirty = false;
}

QVector4D ShaderToyWidget::mouseUniformValue() const
{
    const qreal dpr = devicePixelRatioF();
    const float x = float(m_mousePosition.x() * dpr);
    const float y = float((height() - m_mousePosition.y()) * dpr);
    const float z = m_mousePressed ? float(m_mousePressPosition.x() * dpr) : 0.0f;
    const float w = m_mousePressed ? float((height() - m_mousePressPosition.y()) * dpr) : 0.0f;
    return QVector4D(x, y, z, w);
}

void ShaderToyWidget::reportShaderCompileFailure(const QString &message)
{
    const QString failureKey = currentProgramKey();
    if (!failureKey.isEmpty() && m_lastFailedProgramKey == failureKey) {
        return;
    }

    m_lastFailedProgramKey = failureKey;
    qWarning().noquote() << QStringLiteral("[ShaderToyWidget] %1").arg(message);
    emit shaderCompileFailed(message);
}
