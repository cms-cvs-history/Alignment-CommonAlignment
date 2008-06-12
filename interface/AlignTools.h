#ifndef Alignment_CommonAlignment_AlignTools_H
#define Alignment_CommonAlignment_AlignTools_H

/** \namespace align
 *
 *  Tools for comparing alignables
 *
 *  $Date: 2008/05/05 08:59:57 $
 *  $Revision: 1.3 $
 *  \author Nhan Tran
 */

#include "Alignment/CommonAlignment/interface/Utilities.h"

namespace align{

	///Finds the TR between two alignables - first alignable is reference
	AlgebraicVector diffAlignables(Alignable* refAli, Alignable* curAli, std::string weightBy, bool weightById, std::vector< unsigned int > weightByIdVector);
	
	///Finds the TR between 2 sets of alignables
	///For example, if TIB/TID were to move as one unit
	//void diffAlignables(Alignables& refAlis, Alignables& curAlis, std::vector<AlgebraicVector>& diffs);
	
	///Moves the alignable by the AlgebraicVector
	void moveAlignable(Alignable* ali, AlgebraicVector diff);

	///Creates the points which are used in diffAlignables
	///A set of points corresponding to lowest daughters
	void createPoints(GlobalVectors* Vs, Alignable* ali, std::string weightBy, bool weightById, std::vector< unsigned int > weightByIdVector);
	
	// read module list, return bool
	bool readModuleList( unsigned int, unsigned int, std::vector< unsigned int > );

}

#endif
