
#ifndef GEOMETRY_H
#define GEOMETRY_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#define TWO_PI (M_PI * 2.0)
#define HALF_PI (M_PI / 2.0)
#define QUARTER_PI (M_PI / 4.0)

double dist(double, double);
double dist(double, double, double);
double dist(double, double, double, double);
double dist(double, double, double, double, double, double);
double degrees(double);
double radians(double);
float distf(float, float);
float distf(float, float, float);
float distf(float, float, float, float);
float distf(float, float, float, float, float, float);
float degreesf(float);
float radiansf(float);




//
// double angle(double x, double y);
// double angle(glm::vec2 v);
//
// Angle from X-Axis to vector, range 0 to 360 counterclockwise
//
//       Y
//       |     P
//       |    /
//       |   /
//       |  / \.
//       | / a |
//       |------------------X
//
double angle(double x, double y);
double angle(glm::vec2 v);





//
// double angleBetweenVectors(glm::vec2 a, glm::vec2 b);
// double angleBetweenVectors(glm::vec3 a, glm::vec3 b);
//
// Absolute shortest angle between two vectors, ranging from 0 to 180 in both directions
// Also takes 3-dimensional vectors -> shortest angle between them
//
//                 Y
//       A         |     B
//         .       |    /
//           .     |   /
//             .  _|_ /
//               /a| /
//     ------------|------------------X
//
double angleBetweenVectors(glm::vec2 a, glm::vec2 b);
double angleBetweenVectors(glm::vec3 a, glm::vec3 b);






//
// double angleBetweenVectorsSigned(glm::vec2 a, glm::vec2 b);
//
// Similar to angleBetweenVectors(), but angle is measured from B to A in counterclockwise rotation
// Range: -180 to 180
//
//                 Y
//       A         |     B
//         .       |    /
//           .     |   /
//             .  _|_ /
//               /a| /
//     ------------|------------------X
//
double angleBetweenVectorsSigned(glm::vec2 a, glm::vec2 b);





//
// glm::vec3 getFurthestPointAlongVector(glm::vec3 direction, glm::vec3* points, size_t numberOfPoints);
//
// Returns the furthest point of an array in the specified direction
// Example: returns p[2]
//
//					 p[2]
//                 ^
//                 |     ^ direction
//           p[0]  |    /
//                 |   /
//     p[1]        |  /
//                 | /      p[3]
//     ------------|------------------>
//
glm::vec3 getFurthestPointAlongVector(glm::vec3 direction, glm::vec3* points, glm::size_t numberOfPoints);






//
// glm::vec3 getClosestPointAlongVector(glm::vec3 direction, glm::vec3* points, size_t numberOfPoints);
//
// Returns the closest point of an array in the specified direction
// Example: returns p[1]
//
//					 p[2]
//                 ^
//                 |     ^ direction
//           p[0]  |    /
//                 |   /
//     p[1]        |  /
//                 | /      p[3]
//     ------------|------------------>
//
glm::vec3 getClosestPointAlongVector(glm::vec3 direction, glm::vec3* points, glm::size_t numberOfPoints);



glm::vec2 rotate(glm::vec2 v, float angle);



double angleFromVectorToVector(glm::vec2 a, glm::vec2 b);
double getDirectionalVectorAngle(glm::vec2 v);

#endif // GEOMETRY_H
