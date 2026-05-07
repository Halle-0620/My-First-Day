#include "shadertoywidget.h"

#include <QMouseEvent>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

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

} // namespace

ShaderToyWidget::ShaderToyWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
{
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setMouseTracking(true);
}

ShaderToyWidget::~ShaderToyWidget()
{
    makeCurrent();
    destroyProgram();
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
    update();
}

ShaderEffect ShaderToyWidget::shaderEffect() const
{
    return m_shaderEffect;
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

    if (m_shaderEffect == ShaderEffect::None) {
        return;
    }

    if (!ensureProgram()) {
        return;
    }

    m_program->bind();
    m_vertexArray.bind();
    m_vertexBuffer.bind();

    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2, 0);
    m_program->setUniformValue("iResolution", QVector3D(framebufferWidth, framebufferHeight, 1.0f));
    m_program->setUniformValue("iTime", float(m_elapsedTimer.elapsed()) / 1000.0f);
    m_program->setUniformValue("iMouse", mouseUniformValue());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
    if (m_shaderEffect == ShaderEffect::None) {
        return false;
    }

    if (m_program && m_compiledEffect == m_shaderEffect) {
        return true;
    }

    destroyProgram();

    auto *program = new QOpenGLShaderProgram(this);
    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShaderSource)) {
        delete program;
        return false;
    }

    const QString fragmentSource = fragmentSourceForEffect(m_shaderEffect);
    if (fragmentSource.isEmpty()
        || !program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource)) {
        delete program;
        return false;
    }

    if (!program->link()) {
        delete program;
        return false;
    }

    m_program = program;
    m_compiledEffect = m_shaderEffect;
    return true;
}

QString ShaderToyWidget::fragmentSourceForEffect(ShaderEffect effect) const
{
    switch (effect) {
    case ShaderEffect::DriveHome:
        return QString::fromUtf8(kDriveHomeShaderSource);
    case ShaderEffect::TokyoRain:
        return QString::fromUtf8(kTokyoRainShaderSource);
    case ShaderEffect::None:
        break;
    }

    return {};
}

void ShaderToyWidget::destroyProgram()
{
    delete m_program;
    m_program = nullptr;
    m_compiledEffect = ShaderEffect::None;
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
