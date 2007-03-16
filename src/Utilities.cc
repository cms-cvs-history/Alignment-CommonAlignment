#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Alignment/CommonAlignment/interface/Utilities.h"

align::EulerAngles align::toAngles(const RotationType& rot)
{
  EulerAngles angles(3);

  if ( std::abs( rot.zx() ) < static_cast<Scalar>(1) )
  {
    angles(1) = -std::atan2( rot.zy(), rot.zz() );
    angles(2) =  std::asin( rot.zx() );
    angles(3) = -std::atan2( rot.yx(), rot.xx() );
  }
  else
  {
    edm::LogWarning("Alignment") << "Rounding errors in\n" << rot;

    angles(1) = std::atan2( .5 * ( rot.xy() + rot.yz() ),
			    .5 * ( rot.yy() - rot.xz() ) );
    angles(2) = std::asin( rot.zx() > 0. ? 1. : -1. );
    angles(3) = 0.;
  }

  return angles;
}

align::RotationType align::toMatrix(const EulerAngles& angles)
{
  Scalar s1 = std::sin(angles[0]), c1 = std::cos(angles[0]);
  Scalar s2 = std::sin(angles[1]), c2 = std::cos(angles[1]);
  Scalar s3 = std::sin(angles[2]), c3 = std::cos(angles[2]);

  return RotationType( c2 * c3, c1 * s3 + s1 * s2 * c3, s1 * s3 - c1 * s2 * c3,
	   	      -c2 * s3, c1 * c3 - s1 * s2 * s3, s1 * c3 + c1 * s2 * s3,
		            s2,               -s1 * c2,                c1 * c2);
}

align::PositionType align::motherPosition(const std::vector<const PositionType*>& dauPos)
{
  unsigned int nDau = dauPos.size();

  Scalar posX(0.), posY(0.), posZ(0.); // position of mother

  for (unsigned int i = 0; i < nDau; ++i)
  {
    const PositionType* point = dauPos[i];

    posX += point->x();
    posY += point->y();
    posZ += point->z();
  }

  Scalar inv = 1. / static_cast<Scalar>(nDau);

  return PositionType(posX *= inv, posY *= inv, posZ *= inv);
}

align::RotationType align::diffRot(const GlobalVectors& current,
				   const GlobalVectors& nominal)
{
// Find the matrix needed to rotate the nominal surface to the current one
// using small angle approximation through the equation:
//
//   I * dOmega = dr * r (sum over points)
//
// where dOmega is a vector of small rotation angles about (x, y, z)-axes,
//   and I is the inertia tensor defined as
//
//   I_ij = delta_ij * r^2 - r_i * r_j (sum over points)
//
// delta_ij is the identity matrix. i, j are indices for (x, y, z).
//
// On the rhs of the first eq, r * dr is the cross product of r and dr.
// In this case, r is the nominal vector and dr is the displacement of the
// current point from its nominal point (current vector - nominal vector).
// 
// Since the solution of dOmega (by inverting I) gives angles that are small,
// we rotate the current surface by -dOmega and repeat the process until the
// dOmega^2 is less than a certain tolerance value.
// (In other words, we move the current surface by small angular steps till
// it matches the nominal surface.)
// The full rotation is found by adding up the rotations (given by dOmega)
// in each step. (More precisely, the product of all the matrices.)
//
// Note that, in some cases, if the angular displacement between current and
// nominal is pi, the algorithm can return an identity (no rotation).
// This is because dr = -r and r * dr is all zero.
// This is not a problem since we are dealing with small angles in alignment.

  static const double tolerance = 1e-8;

  RotationType rot; // rotation from nominal to current; init to identity

// Initial values for dr and I; I is always the same in each step

  AlgebraicSymMatrix I(3); // inertia tensor

  GlobalVectors rotated = current; // rotated current vectors in each step

  unsigned int nPoints = nominal.size();

  for (unsigned int j = 0; j < nPoints; ++j)
  {
    const GlobalVector& r = nominal[j];

  // Inertial tensor: I_ij = delta_ij * r^2 - r_i * r_j (sum over points)

    I.fast(1, 1) += r.y() * r.y() + r.z() * r.z();
    I.fast(2, 2) += r.x() * r.x() + r.z() * r.z();
    I.fast(3, 3) += r.y() * r.y() + r.x() * r.x();
    I.fast(2, 1) -= r.x() * r.y(); // row index must be >= col index
    I.fast(3, 1) -= r.x() * r.z();
    I.fast(3, 2) -= r.y() * r.z();
  }

  while (true)
  {
    AlgebraicVector rhs(3); // sum of dr * r

    for (unsigned int j = 0; j < nPoints; ++j)
    {
      const GlobalVector& r = nominal[j];
      const GlobalVector& c = rotated[j];

    // Cross product of dr * r = c * r (sum over points)

      rhs(1) += c.y() * r.z() - c.z() * r.y();
      rhs(2) += c.z() * r.x() - c.x() * r.z();
      rhs(3) += c.x() * r.y() - c.y() * r.x();
    }

    EulerAngles dOmega = CLHEP::solve(I, rhs);

    if (dOmega.normsq() < tolerance) break; // converges, so exit loop

    rot *= toMatrix(dOmega); // add to rotation

  // Not yet converge; move current vectors to new positions and find dr

    for (unsigned int j = 0; j < nPoints; ++j)
    {
      rotated[j] = GlobalVector( rot.multiplyInverse( current[j].basicVector() ) );
    }
  }

  return rot;
}

void align::rectify(RotationType& rot)
{
  rot = toMatrix( toAngles(rot) );
}