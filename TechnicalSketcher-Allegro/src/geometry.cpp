
#include "pch.h"

#include "geometry.h"

double dist(double x, double y, double z) {
	return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

double dist(double x, double y) {
	return sqrt(pow(x, 2) + pow(y, 2));
}

double dist(double x, double y, double _x, double _y) {
	return dist(x - _x, y - _y);
}

double dist(double x, double y, double z, double _x, double _y, double _z) {
	return dist(x - _x, y - _y, z - _z);
}

double degrees(double rad) {
	return rad * 360.0 / TWO_PI;
}

double radians(double deg) {
	return deg * TWO_PI / 360.0;
}

float distf(float x, float y, float z) {
	return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

float distf(float x, float y) {
	return sqrt(pow(x, 2) + pow(y, 2));
}

float distf(float x, float y, float _x, float _y) {
	return distf(x - _x, y - _y);
}

float distf(float x, float y, float z, float _x, float _y, float _z) {
	return distf(x - _x, y - _y, z - _z);
}

float degreesf(float rad) {
	return rad * 360.f / static_cast<float>(TWO_PI);
}

float radiansf(float deg) {
	return deg * static_cast<float>(TWO_PI) / 360.f;
}








double angle(double x, double y) {
	double an = 0;

	if (dist(x, y, 0) > 0) {
		if (y > 0) {
			an = degrees(acos(x / dist(x, y, 0)));
		}
		else if (y <= 0) {
			an = 360 - degrees(acos(x / dist(x, y, 0)));
		}
	}
	else {
		an = 0;
	}

	return an;
}

double angle(glm::vec2 v) {
	return angle(v.x, v.y);
}

double angleBetweenVectors(glm::vec2 a, glm::vec2 b) {
	if(a.length() * b.length() == 0)
		return 0;
	
	return degrees(acos(dot(a, b) / (a.length() * b.length())));
}

double angleBetweenVectors(glm::vec3 a, glm::vec3 b) {
	if(a.length() * b.length() == 0)
		return 0;
	
	return degrees(acos(dot(a, b) / (a.length() * b.length())));
}

double angleBetweenVectorsSigned(glm::vec2 a, glm::vec2 b) {
	return degrees(atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y));
}

glm::vec3 getFurthestPointAlongVector(glm::vec3 direction, glm::vec3* points, size_t numberOfPoints) {

    if(numberOfPoints == 0)
    	return glm::vec3(NAN, NAN, NAN);

	glm::vec3 furthest = points[0];
	double furthestDist = dot(points[0], direction) / direction.length();

    for(size_t i = 0; i < numberOfPoints; i++) {
		double proj = dot(points[i], direction) / direction.length();
      
      if(proj > furthestDist) {
        furthest = points[i];
        furthestDist = proj;
      }
    }
    return furthest;
  }

glm::vec3 getClosestPointAlongVector(glm::vec3 direction, glm::vec3* points, size_t numberOfPoints) {

    if(numberOfPoints == 0)
    	return glm::vec3(NAN, NAN, NAN);

	glm::vec3 furthest = points[0];
	double furthestDist = dot(points[0], direction) / direction.length();

    for(size_t i = 0; i < numberOfPoints; i++) {
		double proj = dot(points[i], direction) / direction.length();
      
      if(proj < furthestDist) {
        furthest = points[i];
        furthestDist = proj;
      }
    }
    return furthest;
  }

double angleFromVectorToVector(glm::vec2 a, glm::vec2 b) {
	double ang = angleBetweenVectorsSigned(b, a);

	if(ang >= 0) {
		return ang;
	} else {
		return 360 + ang;
	}
}

double getDirectionalVectorAngle(glm::vec2 v) {
	if(v.x > 0) {
		if(v.y > 0) {
			glm::vec2 t(1, 0);
			return angleBetweenVectors(v, t);
		} else {
			glm::vec2 t(0, -1);
			return angleBetweenVectors(v, t) + 270;
		}
	} else {
		if(v.y > 0) {
			glm::vec2 t(0, 1);
			return angleBetweenVectors(v, t) + 90;
		} else {
			glm::vec2 t(-1, 0);
			return angleBetweenVectors(v, t) + 180;
		}
	}
}




glm::vec2 rotate(glm::vec2 v, float angle) {
	return glm::vec2(v.x * cos(radians(angle)) - v.y * sin(radians(angle)), 
					 v.x * sin(radians(angle)) + v.y * cos(radians(angle)));
}
