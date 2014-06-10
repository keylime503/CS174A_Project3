//
// raytrace.cpp
//

#define _CRT_SECURE_NO_WARNINGS
#include "matm.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

int g_width;
int g_height;

struct Ray
{
    vec4 origin;
    vec4 dir;
};

/*** My structs ***/

struct RGB {
    
    float red;
    float green;
    float blue;
    
    RGB() : red(0.0), green(0.0), blue(0.0) {}
    
    RGB(float r, float g, float b) : red(r), green(g), blue(b) {}
    
    // Copy Constructor
    RGB(const RGB &p) : red(p.red), green(p.green), blue(p.blue) {}
    
    // Overload addition operators
    RGB operator + (const RGB &p) const { return RGB(red + p.red, green + p.green, blue + p.blue); }
    RGB operator += (const RGB &p) { red += p.red; green += p.green; blue += p.blue; return *this; }
    
} g_back;

struct Intensity {
    
    float red;
    float green;
    float blue;
} g_ambient;

struct Position {
    
    float x;
    float y;
    float z;
};

struct Scale {
    
    float x;
    float y;
    float z;
    
};

struct Sphere {
    
    string name;
    Position pos;
    struct Scale scl; // TODO: why do I need "struct" here? This isn't C!
    RGB color;
    float k_a;
    float k_d;
    float k_s;
    float k_r;
    float n;
    
};

struct Light {
    
    string name;
    Position pos;
    Intensity intensity;
    
};

/*** My global data members ***/

string g_outputFileName;

vector<Sphere> g_spheres;
vector<Light> g_lights;

vector<vec4> g_colors;

float g_left;
float g_right;
float g_top;
float g_bottom;
float g_near;

vec4 eye = vec4(0.0f, 0.0f, 0.0f, 1.0f);

/*** Print Parsed Data for Debugging ***/
void printParsedData() {
    
    cout << "Printing Parsed Data" << endl;
    
    cout << "NEAR: " << g_near << endl;
    cout << "LEFT: " << g_left << endl;
    cout << "RIGHT: " << g_right << endl;
    cout << "BOTTOM: " << g_bottom << endl;
    cout << "TOP: " << g_top << endl;
    cout << "RES: " << g_width << ", " << g_height << endl;
    
    Sphere s;
    size_t nSpheres = g_spheres.size();
    cout << nSpheres << " spheres:" << endl;
    for (int i = 0; i < nSpheres; i++) {
        
        s = g_spheres[i];
        cout << "\t" << s.name << ": pos(" << s.pos.x << ", " << s.pos.y << ", " << s.pos.z << "), ";
        cout << "scl(" << s.scl.x << ", " << s.scl.y << ", " << s.scl.z << "), ";
        cout << "color(" << s.color.red << ", " << s.color.green << ", " << s.color.blue << "), ";
        cout << s.k_a << ", ";
        cout << s.k_d << ", ";
        cout << s.k_s << ", ";
        cout << s.k_r << ", ";
        cout << s.n << endl;
    }
    
    Light l;
    size_t nLights = g_lights.size();
    cout << nLights << " lights:" << endl;
    for (int i = 0; i < nLights; i++) {
        
        l = g_lights[i];
        cout << "\t" << l.name << ": pos(" << l.pos.x << ", " << l.pos.y << ", " << l.pos.z << "), ";
        cout << "scl(" << l.intensity.red << ", " << l.intensity.green << ", " << l.intensity.blue << ")" << endl;;
    }
    
    cout << "BACK: (" << g_back.red << ", " << g_back.green << ", " << g_back.blue << ")" << endl;
    cout << "AMBIENT: (" << g_ambient.red << ", " << g_ambient.green << ", " << g_ambient.blue << ")" << endl;
    cout << "OUTPUT: " << g_outputFileName << endl;

    return;
}


// -------------------------------------------------------------------
// Input file parsing

vec4 toVec4(const string& s1, const string& s2, const string& s3)
{
    stringstream ss(s1 + " " + s2 + " " + s3);
    vec4 result;
    ss >> result.x >> result.y >> result.z;
    result.w = 1.0f;
    return result;
}

float toFloat(const string& s)
{
    stringstream ss(s);
    float f;
    ss >> f;
    return f;
}

void parseLine(const vector<string>& vs)
{
    if (vs[0] == "RES") {
        
        g_width = (int)toFloat(vs[1]);
        g_height = (int)toFloat(vs[2]);
        g_colors.resize(g_width * g_height);
        
    } else if (vs[0] == "NEAR") {
        
        g_near = toFloat(vs[1]);
        
    } else if (vs[0] == "LEFT") {
        
        g_left = toFloat(vs[1]);
        
    } else if (vs[0] == "RIGHT") {
        
        g_right = toFloat(vs[1]);
        
    } else if (vs[0] == "BOTTOM") {
        
        g_bottom = toFloat(vs[1]);
        
    } else if (vs[0] == "TOP") {
        
        g_top = toFloat(vs[1]);
        
    } else if (vs[0] == "SPHERE") {
        
        if (g_spheres.size() < 5) {
            
            Sphere sphere;
            sphere.name = vs[1];
            sphere.pos.x = toFloat(vs[2]);
            sphere.pos.y = toFloat(vs[3]);
            sphere.pos.z = toFloat(vs[4]);
            sphere.scl.x = toFloat(vs[5]);
            sphere.scl.y = toFloat(vs[6]);
            sphere.scl.z = toFloat(vs[7]);
            sphere.color.red = toFloat(vs[8]);
            sphere.color.green = toFloat(vs[9]);
            sphere.color.blue = toFloat(vs[10]);
            sphere.k_a = toFloat(vs[11]);
            sphere.k_d = toFloat(vs[12]);
            sphere.k_s = toFloat(vs[13]);
            sphere.k_r = toFloat(vs[14]);
            sphere.n = toFloat(vs[15]);

            g_spheres.push_back(sphere);
            
        } else {
            
            cout << "Cannot have more than 5 spheres" << endl;
            exit(1);
            
        }
        
    } else if (vs[0] == "LIGHT") {
        
        if (g_lights.size() < 5) {
            
            Light light;
            light.name = vs[1];
            light.pos.x = toFloat(vs[2]);
            light.pos.y = toFloat(vs[3]);
            light.pos.z = toFloat(vs[4]);
            light.intensity.red = toFloat(vs[5]);
            light.intensity.green = toFloat(vs[6]);
            light.intensity.blue = toFloat(vs[7]);
            
            g_lights.push_back(light);

        } else {
            
            cout << "Cannot have more than 5 lights" << endl;
            exit(1);
            
        }
        
    } else if (vs[0] == "BACK") {
        
        g_back.red = toFloat(vs[1]);
        g_back.green = toFloat(vs[2]);
        g_back.blue = toFloat(vs[3]);
        
    } else if (vs[0] == "AMBIENT") {
        
        g_ambient.red = toFloat(vs[1]);
        g_ambient.green = toFloat(vs[2]);
        g_ambient.blue = toFloat(vs[3]);
        
    } else if (vs[0] == "OUTPUT") {
        
        g_outputFileName = vs[1];
    }
}

void loadFile(const char* filename)
{
    ifstream is(filename);
    if (is.fail())
    {
        cout << "Could not open file " << filename << endl;
        exit(1);
    }
    string s;
    vector<string> vs;
    while(!is.eof())
    {
        vs.clear();
        getline(is, s);
        istringstream iss(s);
        while (!iss.eof())
        {
            string sub;
            iss >> sub;
            vs.push_back(sub);
        }
        parseLine(vs);
    }
}


// -------------------------------------------------------------------
// Utilities

void setColor(int ix, int iy, const vec4& color)
{
    int iy2 = g_height - iy - 1; // Invert iy coordinate.
    g_colors[iy2 * g_width + ix] = color;
}


// -------------------------------------------------------------------
// Intersection routine

mat4 getSphereTransMatrix(Sphere s) {
    
    mat4 m = mat4();
    m *= Translate(s.pos.x, s.pos.y, s.pos.z);
    m *= Scale(s.scl.x, s.scl.y, s.scl.z);
    return m;
}

mat4 getSphereInverseTransMatrix(Sphere s) {
    
    mat4 m = getSphereTransMatrix(s);
    mat4 mInv;
    bool result = InvertMatrix(m, mInv);
    if (result == false) {
        
        cout << "Matrix not invertible" << endl;
        exit(1);
        
    }
    
    return mInv;
}

RGB diffuseLight(Light l, vec4 p, Sphere pointSphere) {
    
    // Calculate vector from point to light
    vec4 lightPos = vec4(l.pos.x, l.pos.y, l.pos.z, 1.0f);
    vec4 lightVector = normalize(lightPos - p);
    
    // Calculate Normal
    vec4 spherePos = vec4(pointSphere.pos.x, pointSphere.pos.y, pointSphere.pos.z, 1.0f);
    vec4 normal = normalize(p - spherePos);
    
    // Calculate dot product
    float NdotL = dot(normal, lightVector);
    
    float diffuseRed = pointSphere.k_d * l.intensity.red * NdotL * pointSphere.color.red;
    float diffuseGreen = pointSphere.k_d * l.intensity.green * NdotL * pointSphere.color.green;
    float diffuseBlue = pointSphere.k_d * l.intensity.blue * NdotL * pointSphere.color.blue;
    
    RGB diffuseRGB = RGB(diffuseRed, diffuseGreen, diffuseBlue);
    return diffuseRGB;
}

RGB specularLight(Light l, vec4 p, Sphere pointSphere) {
    
    return RGB();
}

RGB shadowRay(Light l, vec4 p, Sphere pointSphere) {
    
    vec4 lightPos = vec4(l.pos.x, l.pos.y, l.pos.z, 1.0f);
    vec4 rayToLight = lightPos - p;

    // Determine if light is blocked by other spheres
    bool blocked = false;
    Sphere s;
    size_t nSpheres = g_spheres.size();
    for (int i = 0; i < nSpheres; i++) {
        
        s = g_spheres[i];
        
        mat4 mInv = getSphereInverseTransMatrix(s);
        vec4 trans_origin = mInv * p;;
        vec4 trans_dir = mInv * rayToLight;
        
        // Calculate discriminant
        float t = -1.0; // units of "dir" vector from eye to first intersection point with s
        float d = (dot(trans_dir, trans_origin) * dot(trans_dir, trans_origin)) - (dot(trans_dir, trans_dir) * (dot(trans_origin, trans_origin) - 2.0));
        if (d < 0.0) {
            
            /* No solution => no intersection */
            
            continue;
            
        } else if (d == 0.0) {
            
            /* One solution => One Intersection */
            
            t = (- dot(trans_dir, trans_origin)) / (dot(trans_dir, trans_dir));
            
        } else {
            
            /* Two solutions => Two intersections */
            
            float t1 = (- dot(trans_dir, trans_origin) + sqrt(d)) / (dot(trans_dir, trans_dir));
            float t2 = (- dot(trans_dir, trans_origin) - sqrt(d)) / (dot(trans_dir, trans_dir));
            t = min(t1, t2);
            
        }
        
        // Check if discovered intersection is actually blocking the light source
        if (t < 1.0 && t > 0.0001) {
            
            blocked = true;
            break;
        }
    }
    
    RGB ret = RGB(0.0, 0.0, 0.0);
    
    if (!blocked) {
        
        // Calculate local illumination at this point from this light
        ret += diffuseLight(l, p, pointSphere);
        ret += specularLight(l, p, pointSphere);
        
    }
    
    return ret;
    
    // TODO: Determine if this sphere itself is blocking the light, do I already do this?
}

// Callers of this function should check for -1.0 return value
RGB getClosestIntersection(const Ray& ray) {
    
    Sphere s;
    Sphere closestSphere;
    float tMax = -1.0;
    size_t nSpheres = g_spheres.size();
    for (int i = 0; i < nSpheres; i++) {
        
        s = g_spheres[i];
        
        mat4 mInv = getSphereInverseTransMatrix(s);
        vec4 trans_eye = mInv * eye;
        vec4 trans_dir = mInv * ray.dir;
        
        // Calculate discriminant
        float t = -1.0; // units of "dir" vector from eye to first intersection point with s
        float d = (dot(trans_dir, trans_eye) * dot(trans_dir, trans_eye)) - (dot(trans_dir, trans_dir) * (dot(trans_eye, trans_eye) - 2.0));
        if (d < 0.0) {
            
            /* No solution => no intersection, use background color */
            
            continue;
            
        } else if (d == 0.0) {
            
            /* One solution => One Intersection */
            
            t = (- dot(trans_dir, trans_eye)) / (dot(trans_dir, trans_dir));
            
        } else {
            
            /* Two solutions => Two intersections */
            
            float t1 = (- dot(trans_dir, trans_eye) + sqrt(d)) / (dot(trans_dir, trans_dir));
            float t2 = (- dot(trans_dir, trans_eye) - sqrt(d)) / (dot(trans_dir, trans_dir));
            t = min(t1, t2);
            
        }
        
        // Check if this intersection is the closest to the camera so far
        if (tMax == -1.0 || t < tMax) {
            
            if (t > 1.0) {
                
                tMax = t;
                // TODO: optimize by doing &g_spheres[need index of s here]
                closestSphere = s;
            
            }
        }
    }
    
    // Check if ray does not intersect any objects
    if (tMax == -1.0) {
        
        return g_back;
    }
    
    // Calculate intersection point
    vec4 p = eye + ray.dir * tMax;
    
    RGB localColor = RGB();
    
    // Loop through lights and compute local color addition for shadow rays
    size_t nLights = g_lights.size();
    for (int i = 0; i < nLights; i++) {
        
        localColor += shadowRay(g_lights[i], p, closestSphere);
        //RGB tempRGB = shadowRay(g_lights[i], p);
        //cout << "(" << tempRGB.red << ", " << tempRGB.green << ", " << tempRGB.blue << ")" << endl;
        
    }
    
    // Computer Ambient Light Contribution
    float ambientRed = closestSphere.k_a * g_ambient.red * closestSphere.color.red;
    float ambientGreen = closestSphere.k_a * g_ambient.green * closestSphere.color.green;
    float ambientBlue = closestSphere.k_a * g_ambient.blue * closestSphere.color.blue;
    RGB ambientColor = RGB(ambientRed, ambientGreen, ambientBlue);
    
    RGB pixelColor = localColor + ambientColor; // TODO: change this to add other sources of light by slide 2 on page 12 of lecture 12
    // TODO: See spec equation for full pixel color equation (ambient, diffuse, specular, and reflection)
    
    return pixelColor;
}


// -------------------------------------------------------------------
// Ray tracing

vec4 trace(const Ray& ray)
{
    // TODO: implement your ray tracing routine here.
    
    // Set color based on first intersection with object. If ray doesn't intersect with any objects, getClosestIntersection() returns background color
    RGB closestColor = getClosestIntersection(ray);
    return vec4(closestColor.red, closestColor.green, closestColor.blue, 1.0f);
}

vec4 getDir(int ix, int iy)
{
    // TODO: modify this. This should return the direction from the origin
    // to pixel (ix, iy), normalized. -- Done
    
    float u_c = g_left + (g_right - g_left) * (ix / (g_width - 1.0));
    float v_r = g_bottom + (g_top - g_bottom) * (iy / (g_height - 1.0));
    float n = -g_near;
    
    vec4 dir;
    dir = vec4(u_c, v_r, n, 0.0f);
    return normalize(dir);
}

void renderPixel(int ix, int iy)
{
    Ray ray;
    ray.origin = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ray.dir = getDir(ix, iy);
    vec4 color; // TODO: change back to "... = trace(ray);"
    
    
    // TODO: debugging code, remove this
//    if (ix != 300 || iy != 300) {
//        
//        color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
//        
//    } else {
    
        color = trace(ray);
//    }
    
    setColor(ix, iy, color);
}

void render()
{
    for (int iy = 0; iy < g_height; iy++)
        for (int ix = 0; ix < g_width; ix++)
            renderPixel(ix, iy);
}


// -------------------------------------------------------------------
// PPM saving

void savePPM(int Width, int Height, string fname_str, unsigned char* pixels)
{
    const char * fname = fname_str.c_str();
    
    FILE *fp;
    const int maxVal=255;

    printf("Saving image %s: %d x %d\n", fname, Width, Height);
    fp = fopen(fname,"wb");
    if (!fp) {
        printf("Unable to open file '%s'\n", fname);
        return;
    }
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", Width, Height);
    fprintf(fp, "%d\n", maxVal);

    for(int j = 0; j < Height; j++) {
        fwrite(&pixels[j*Width*3], 3, Width, fp);
    }

    fclose(fp);
}

void saveFile()
{
    // Convert color components from floats to unsigned chars.
    // TODO: clamp values if out of range.
    unsigned char* buf = new unsigned char[g_width * g_height * 3];
    for (int y = 0; y < g_height; y++)
        for (int x = 0; x < g_width; x++)
            for (int i = 0; i < 3; i++)
                buf[y*g_width*3+x*3+i] = (unsigned char)(((float*)g_colors[y*g_width+x])[i] * 255.9f);
    
    // TODO: change file name based on input file name. -- Done
    savePPM(g_width, g_height, g_outputFileName, buf);
    delete[] buf;
}


// -------------------------------------------------------------------
// Main

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Usage: template-rt <input_file.txt>" << endl;
        exit(1);
    }
    loadFile(argv[1]);
    render();
    saveFile();
	return 0;
}

