
#include "../shared/app.h"

struct StrokeOptions {
  double size;
  double thinning;
  double smoothing;
  double streamline;
  double (*easing)(double distance);
  bool simulatePressure;
  struct {
    bool cap;
    double taper;
    double (*easing)(double distance);
  } start;
  struct {
    bool cap;
    double taper;
    double (*easing)(double distance);
  } end;
  // Whether to handle the points as a completed stroke.
  bool last;
};

/**
 * The points returned by `getStrokePoints`, and the input for `getStrokeOutlinePoints`.
 */
struct StrokePoint {
  Vec2 point;
  double pressure;
  double distance;
  Vec2 vector;
  double runningLength;
};

/**
 * Interpolate vector A to B with a scalar t
 * @param A
 * @param B
 * @param t scalar
 * @internal
 */
Vec2 lrp(Vec2 A, Vec2 B, double t)
{
  return A + (B - A) * t;
}
InterpolationPoint lrp(InterpolationPoint A, InterpolationPoint B, double t)
{
  return {
    .pos = A.pos + (B.pos - A.pos) * t,
    .thickness = A.thickness + (B.thickness - A.thickness) * (t / (B.pos - A.pos).length()),
  };
}

/**
 * ## getStrokePoints
 * @description Get an array of points as objects with an adjusted point, pressure, vector, distance, and
 runningLength.
 * @param points An array of points (as `[x, y, pressure]` or `{x, y, pressure}`). Pressure is optional in both
 cases.
 * @param options (optional) An object with options.
 * @param options.size	The base size (diameter) of the stroke.
 * @param options.thinning The effect of pressure on the stroke's size.
 * @param options.smoothing	How much to soften the stroke's edges.
 * @param options.easing	An easing function to apply to each point's pressure.
 * @param options.simulatePressure Whether to simulate pressure based on velocity.
 * @param options.start Cap, taper and easing for the start of the line.
 * @param options.end Cap, taper and easing for the end of the line.
 * @param options.last Whether to handle the points as a completed stroke.
 */
List<StrokePoint> getStrokePoints(Arena& arena, List<InterpolationPoint> points, StrokeOptions options)
{
  bool isComplete = options.last;

  // If we don't have any points, return an empty array.
  if (points.length == 0)
    return {};

  // Find the interpolation level between points.
  auto t = 0.15 + (1 - options.streamline) * 0.85;

  // Whatever the input is, make sure that the points are in number[][].
  List<InterpolationPoint> pts;
  for (auto p : points) {
    pts.push(arena, p);
  }

  // Add extra points between the two, to help avoid "dash" lines
  // for strokes with tapered start and ends. Don't mutate the
  // input array!
  if (pts.length == 2) {
    auto last = pts[1];
    // pts = pts.slice(0, -1);
    pts.pop();
    for (auto i = 1; i < 5; i++) {
      pts.push(arena, lrp(pts[0], last, i / 4.0));
    }
  }

  // If there's only one point, add another point at a 1pt offset.
  // Don't mutate the input array!
  if (pts.length == 1) {
    pts.push(arena, { .pos = pts[0].pos + Vec2(1, 1), .thickness = pts[0].thickness });
  }

  // The strokePoints array will hold the points for the stroke.
  // Start it out with the first point, which needs no adjustment.
  List<StrokePoint> strokePoints;
  strokePoints.push(arena,
      {
          .point = pts[0].pos,
          .pressure = pts[0].thickness >= 0 ? pts[0].thickness : 0.25,
          .distance = 0,
          .vector = Vec2(1, 1),
          .runningLength = 0,
      });

  // A flag to see whether we've already reached out minimum length
  auto hasReachedMinimumLength = false;

  // We use the runningLength to keep track of the total distance
  double runningLength = 0;

  // We're set this to the latest point, so we can use it to calculate
  // the distance and vector of the next point.
  auto prev = strokePoints[0];

  auto max = pts.length - 1;

  // Iterate through all of the points, creating StrokePoints.
  for (auto i = 1; i < pts.length; i++) {
    auto point = isComplete && i == max ? // If we're at the last point, and `options.last` is true,
                                          // then add the actual input point.
        pts[i].pos
                                        : // Otherwise, using the t calculated from the streamline
                                          // option, interpolate a new point between the previous
                                          // point the current point.
        lrp(prev.point, pts[i].pos, t);

    // If the new point is the same as the previous point, skip ahead.
    if (prev.point == point)
      continue;

    // How far is the new point from the previous point?
    auto distance = (point - prev.point).length();

    // Add this distance to the total "running length" of the line.
    runningLength += distance;

    // At the start of the line, we wait until the new point is a
    // certain distance away from the original point, to avoid noise
    if (i < max && !hasReachedMinimumLength) {
      if (runningLength < options.size)
        continue;
      hasReachedMinimumLength = true;
      // TODO: Backfill the missing points so that tapering works correctly.
    }
    // Create a new strokepoint (it will be the new "previous" one).
    prev = {
      // The adjusted point
      .point = point,
      // The input pressure (or .5 if not specified)
      .pressure = pts[i].thickness >= 0 ? pts[i].thickness : 0.5,
      // The distance between the current point and the previous point
      .distance = distance,
      // The vector from the current point to the previous point
      .vector = (prev.point - point).normalize(),
      // The total distance so far
      .runningLength = runningLength,
    };

    // Push it to the strokePoints array.
    strokePoints.push(arena, prev);
  }

  // Set the vector of the first point to be the same as the second point.
  if (strokePoints.length > 1) {
    strokePoints[0].vector = strokePoints[1].vector;
  } else {
    strokePoints[0].vector = Vec2(0, 0);
  }

  return strokePoints;
}

// This is the rate of change for simulated pressure. It could be an option.
auto RATE_OF_PRESSURE_CHANGE = 0.275;

// Browser strokes seem to be off if PI is regular, a tiny offset seems to fix it
auto FIXED_PI = M_PI + 0.0001;

/**
 * Perpendicular rotation of a vector A
 * @param A
 * @internal
 */
Vec2 per(Vec2 A)
{
  return Vec2(A.y, -A.x);
}

/**
 * Dot product
 * @param A
 * @param B
 * @internal
 */
double dpr(Vec2 A, Vec2 B)
{
  return A.x * B.x + A.y * B.y;
}

/**
 * Rotate a vector around another vector by r (radians)
 * @param A vector
 * @param C center
 * @param r rotation in radians
 * @internal
 */
Vec2 rotAround(Vec2 A, Vec2 C, double r)
{
  auto s = sin(r);
  auto c = cos(r);

  auto px = A.x - C.x;
  auto py = A.y - C.y;

  auto nx = px * c - py * s;
  auto ny = px * s + py * c;

  return Vec2(nx + C.x, ny + C.y);
}

/**
 * Project a point A in the direction B by a scalar c
 * @param A
 * @param B
 * @param c
 * @internal
 */
Vec2 prj(Vec2 A, Vec2 B, double c)
{
  return (A + (B * c));
}

/**
 * Compute a radius based on the pressure.
 * @param size
 * @param thinning
 * @param pressure
 * @param easing
 * @internal
 */
double getStrokeRadius(double size, double thinning, double pressure, double (*easing)(double distance))
{
  return size * easing(0.5 - thinning * (0.5 - pressure));
}

/**
 * ## getStrokeOutlinePoints
 * @description Get an array of points (as `[x, y]`) representing the outline of a stroke.
 * @param points An array of StrokePoints as returned from `getStrokePoints`.
 * @param options (optional) An object with options.
 * @param options.size	The base size (diameter) of the stroke.
 * @param options.thinning The effect of pressure on the stroke's size.
 * @param options.smoothing	How much to soften the stroke's edges.
 * @param options.easing	An easing function to apply to each point's pressure.
 * @param options.simulatePressure Whether to simulate pressure based on velocity.
 * @param options.start Cap, taper and easing for the start of the line.
 * @param options.end Cap, taper and easing for the end of the line.
 * @param options.last Whether to handle the points as a completed stroke.
 */
List<Vec2> getStrokeOutlinePoints(Arena& arena, List<StrokePoint> points, StrokeOptions options)
{
  auto isComplete = options.last;

  if (!options.start.easing) {
    options.start.easing = [](double t) { return t * (2 - t); };
  }
  if (!options.end.easing) {
    options.end.easing = [](double t) {
      t--;
      return t * t * t + 1;
    };
  }

  // We can't do anything with an empty array or a stroke with negative size.
  if (points.length == 0 || options.size <= 0) {
    return {};
  }

  // The total length of the line
  auto totalLength = points[points.length - 1].runningLength;

  auto taperStart = options.start.taper == false ? 0
      : options.start.taper == true              ? max(options.size, totalLength)
                                                 : (options.start.taper);

  auto taperEnd = options.end.taper == false ? 0
      : options.end.taper == true            ? max(options.size, totalLength)
                                             : (options.end.taper);

  // The minimum allowed distance between points (squared)
  auto minDistance = pow(options.size * options.smoothing, 2);

  // Our collected left and right points
  List<Vec2> leftPts;
  List<Vec2> rightPts;

  // Previous pressure (start with average of first five pressures,
  // in order to prevent fat starts for every line. Drawn lines
  // almost always start slow!
  double prevPressure = points[0].pressure;
  for (size_t i = 1; i < min(10, points.length); ++i) {
    double pressure = points[i].pressure;

    if (options.simulatePressure) {
      double sp = min(1.0f, points[i].distance / options.size);
      double rp = min(1.0f, 1.0f - sp);
      pressure = min(1.0f, prevPressure + (rp - prevPressure) * (sp * RATE_OF_PRESSURE_CHANGE));
    }

    prevPressure = (prevPressure + pressure) / 2.0f;
  }

  // The current radius
  auto radius = getStrokeRadius(options.size, options.thinning, points[points.length - 1].pressure, options.easing);

  // The radius of the first saved point
  double firstRadius = -1.f;

  // Previous vector
  auto prevVector = points[0].vector;

  // Previous left and right points
  auto pl = points[0].point;
  auto pr = pl;

  // Temporary left and right points
  auto tl = pl;
  auto tr = pr;

  // Keep track of whether the previous point is a sharp corner
  // ... so that we don't detect the same corner twice
  auto isPrevPointSharpCorner = false;

  // let short = true

  /*
    Find the outline's left and right points

    Iterating through the points and populate the rightPts and leftPts arrays,
    skipping the first and last pointsm, which will get caps later on.
  */

  for (size_t i = 0; i < points.length; i++) {
    auto pressure = points[i].pressure;
    auto point = points[i].point;
    auto vector = points[i].vector;
    auto distance = points[i].distance;
    auto runningLength = points[i].runningLength;

    // Removes noise from the end of the line
    if (i < points.length - 1 && totalLength - runningLength < 3) {
      continue;
    }

    /*
      Calculate the radius

      If not thinning, the current point's radius will be half the size; or
      otherwise, the size will be based on the current (real or simulated)
      pressure.
    */

    if (options.thinning) {
      if (options.simulatePressure) {
        // If we're simulating pressure, then do so based on the distance
        // between the current point and the previous point, and the size
        // of the stroke. Otherwise, use the input pressure.
        auto sp = min(1, distance / options.size);
        auto rp = min(1, 1 - sp);
        pressure = min(1, prevPressure + (rp - prevPressure) * (sp * RATE_OF_PRESSURE_CHANGE));
      }

      radius = getStrokeRadius(options.size, options.thinning, pressure, options.easing);
    } else {
      radius = options.size / 2.0;
    }

    if (firstRadius == -1.f) {
      firstRadius = radius;
    }

    /*
      Apply tapering

      If the current length is within the taper distance at either the
      start or the end, calculate the taper strengths. Apply the smaller
      of the two taper strengths to the radius.
    */

    auto ts = runningLength < taperStart ? options.start.easing(runningLength / taperStart) : 1;

    auto te = totalLength - runningLength < taperEnd ? options.end.easing((totalLength - runningLength) / taperEnd) : 1;

    radius = max(0.01, radius * min(ts, te));

    /* Add points to left and right */

    /*
      Handle sharp corners

      Find the difference (dot product) between the current and next vector.
      If the next vector is at more than a right angle to the current vector,
      draw a cap at the current point.
    */

    auto nextVector = (i < points.length - 1 ? points[i + 1] : points[i]).vector;
    auto nextDpr = i < points.length - 1 ? dpr(vector, nextVector) : 1.0;
    auto prevDpr = dpr(vector, prevVector);

    auto isPointSharpCorner = prevDpr < 0 && !isPrevPointSharpCorner;
    auto isNextPointSharpCorner = nextDpr < 0;

    if (isPointSharpCorner || isNextPointSharpCorner) {
      // It's a sharp corner. Draw a rounded cap and move on to the next point
      // Considering saving these and drawing them later? So that we can avoid
      // crossing future points.

      auto offset = per(prevVector) * radius;

      for (double step = 1 / 13.0, t = 0; t <= 1; t += step) {
        tl = rotAround((point - offset), point, FIXED_PI * t);
        leftPts.push(arena, tl);

        tr = rotAround((point + offset), point, FIXED_PI * -t);
        rightPts.push(arena, tr);
      }

      pl = tl;
      pr = tr;

      if (isNextPointSharpCorner) {
        isPrevPointSharpCorner = true;
      }
      continue;
    }

    isPrevPointSharpCorner = false;

    // Handle the last point
    if (i == points.length - 1) {
      auto offset = per(vector) * radius;
      leftPts.push(arena, (point - offset));
      rightPts.push(arena, (point + offset));
      continue;
    }

    /*
      Add regular points

      Project points to either side of the current point, using the
      calculated size as a distance. If a point's distance to the
      previous point on that side greater than the minimum distance
      (or if the corner is kinda sharp), add the points to the side's
      points array.
    */

    auto offset = (per(lrp(nextVector, vector, nextDpr)) * radius);

    tl = (point - offset);

    if (i <= 1 || pow((pl - tl).length(), 2) > minDistance) {
      leftPts.push(arena, tl);
      pl = tl;
    }

    tr = (point + offset);

    if (i <= 1 || pow((pr - tr).length(), 2) > minDistance) {
      rightPts.push(arena, tr);
      pr = tr;
    }

    // Set variables for next iteration
    prevPressure = pressure;
    prevVector = vector;
  }

  /*
    Drawing caps

    Now that we have our points on either side of the line, we need to
    draw caps at the start and end. Tapered lines don't have caps, but
    may have dots for very short lines.
  */

  auto firstPoint = points[0].point;

  auto lastPoint = points.length > 1 ? points[points.length - 1].point : (points[0].point + Vec2(1, 1));

  List<Vec2> startCap;

  List<Vec2> endCap;

  /*
    Draw a dot for very short or completed strokes

    If the line is too short to gather left or right points and if the line is
    not tapered on either side, draw a dot. If the line is tapered, then only
    draw a dot if the line is both very short and complete. If we draw a dot,
    we can just return those points.
  */

  if (points.length == 1) {
    if (!(taperStart || taperEnd) || isComplete) {
      auto start = prj(firstPoint, per(firstPoint - lastPoint).normalize(), -(firstRadius || radius));
      List<Vec2> dotPts;
      for (auto step = 1 / 13.0, t = step; t <= 1; t += step) {
        dotPts.push(arena, rotAround(start, firstPoint, FIXED_PI * 2 * t));
      }
      return dotPts;
    }
  } else {
    /*
    Draw a start cap

    Unless the line has a tapered start, or unless the line has a tapered end
    and the line is very short, draw a start cap around the first point. Use
    the distance between the second left and right point for the cap's radius.
    Finally remove the first left and right points. :psyduck:
  */

    if (taperStart || (taperEnd && points.length == 1)) {
      // The start point is tapered, noop
    } else if (options.start.cap) {
      // Draw the round cap - add thirteen points rotating the right point around the start point to the left point
      for (auto step = 1 / 13.0, t = step; t <= 1; t += step) {
        auto pt = rotAround(rightPts[0], firstPoint, FIXED_PI * t);
        startCap.push(arena, pt);
      }
    } else {
      // Draw the flat cap - add a point to the left and right of the start point
      auto cornersVector = leftPts[0] - rightPts[0];
      auto offsetA = cornersVector * 0.5;
      auto offsetB = cornersVector * 0.51;

      startCap.push(arena, (firstPoint - offsetA));
      startCap.push(arena, (firstPoint - offsetB));
      startCap.push(arena, (firstPoint + offsetB));
      startCap.push(arena, (firstPoint + offsetA));
    }

    /*
    Draw an end cap

    If the line does not have a tapered end, and unless the line has a tapered
    start and the line is very short, draw a cap around the last point. Finally,
    remove the last left and right points. Otherwise, add the last point. Note
    that This cap is a full-turn-and-a-half: this prevents incorrect caps on
    sharp end turns.
  */

    auto direction = per(-points[points.length - 1].vector);

    if (taperEnd || (taperStart && points.length == 1)) {
      // Tapered end - push the last point to the line
      endCap.push(arena, lastPoint);
    } else if (options.end.cap) {
      // Draw the round end cap
      auto start = prj(lastPoint, direction, radius);
      for (auto step = 1 / 29.0, t = step; t < 1; t += step) {
        endCap.push(arena, rotAround(start, lastPoint, FIXED_PI * 3 * t));
      }
    } else {
      // Draw the flat end cap

      endCap.push(arena, (lastPoint + (direction * radius)));
      endCap.push(arena, (lastPoint + (direction * radius * 0.99)));
      endCap.push(arena, (lastPoint - (direction * radius * 0.99)));
      endCap.push(arena, (lastPoint - (direction * radius)));
    }
  }

  /*
    Return the points in the correct winding order: begin on the left side, then
    continue around the end cap, then come back along the right side, and finally
    complete the start cap.
  */

  List<Vec2> result;

  // Append leftPts
  for (auto& pt : leftPts)
    result.push(arena, pt);

  // Append endCap
  for (auto& pt : endCap)
    result.push(arena, pt);

  // Append reversed rightPts
  auto reversedRight = rightPts.reverse(arena);
  for (auto& pt : reversedRight)
    result.push(arena, pt);

  // Append startCap
  for (auto& pt : startCap)
    result.push(arena, pt);

  return result;

  //   return leftPts.concat(endCap, rightPts.reverse(arena), startCap);
}

List<Vec2> getStroke(Arena& arena, List<InterpolationPoint> points, StrokeOptions options)
{
  return getStrokeOutlinePoints(arena, getStrokePoints(arena, points, options), options);
}