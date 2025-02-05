// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file   MID/Base/src/Mapping.cxx
/// \brief  Implementation of mapping for MID
/// \author Diego Stocco <Diego.Stocco at cern.ch>
/// \date   11 December 2017
#include "MIDBase/Mapping.h"

#include <cassert>
#include <iostream>
#include "MIDBase/Constants.h"

namespace o2
{
namespace mid
{
//______________________________________________________________________________
Mapping::Mapping() : mDetectionElements(), mBoardIndexes()
{
  /// Default constructor
  init();
}

//______________________________________________________________________________
void Mapping::init()
{
  /// Initializes mapping

  buildDETypeLarge(0, {{1, 17, 39, 61, 77, 93, 109}});
  buildDETypeMedium(1, {{2, 18, 40, 62, 78, 94, 110}}, true);
  buildDETypeMedium(2, {{4, 20, 42, 64, 80, 96, 111}}, false);
  buildDETypeCut(3, {{6, 22, 44, 66, 82, 98, 112}}, true);
  buildDETypeShort(4, {{0, 26, 48, 68, 84, 100, 113}});
  buildDETypeCut(5, {{9, 30, 52, 70, 86, 102, 114}}, false);
  buildDETypeMedium(6, {{12, 34, 56, 72, 88, 104, 115}}, false);
  buildDETypeMedium(7, {{14, 36, 58, 74, 90, 106, 116}}, true);
  buildDETypeLarge(8, {{16, 38, 60, 76, 92, 108, 117}});
}

//______________________________________________________________________________
void Mapping::setupSegmentation(int rpcType, int column, int nStripsNBP, int stripPitchNBP, int nBoardsBP,
                                int firstBoardId, bool isBelowBeamPipe)
{
  /// Initializes column segmentation
  MpColumn& columnStruct{mDetectionElements[rpcType].columns[column]};
  columnStruct.nStripsNBP = nStripsNBP;
  columnStruct.stripPitchNBP = stripPitchNBP;
  columnStruct.stripPitchBP = (nBoardsBP > 0) ? 4 / nBoardsBP : 0;
  int nBoardsBPtot = nBoardsBP;
  if (nBoardsBP == 3) {
    nBoardsBPtot = 4;
    if (!isBelowBeamPipe) {
      --firstBoardId;
    }
  }

  for (int iboard = 0; iboard < nBoardsBPtot; ++iboard) {
    int boardIdx = firstBoardId + iboard - 1;
    if (nBoardsBP == 3) {
      if ((isBelowBeamPipe && iboard == 3) || (!isBelowBeamPipe && iboard == 0)) {
        boardIdx = 117;
      }
    }
    columnStruct.boardsBP.push_back(boardIdx);
    mBoardIndexes[boardIdx].deType = (boardIdx < 117) ? rpcType : 0;
    mBoardIndexes[boardIdx].column = (boardIdx < 117) ? column : 0;
    mBoardIndexes[boardIdx].line = (boardIdx < 117) ? iboard : 0;
  }
}

//______________________________________________________________________________
void Mapping::setupSegmentationLastColumn(int rpcType, int boardId)
{
  /// Initializes the segmentation of last column
  /// it is the same in all RPCs
  setupSegmentation(rpcType, 6, 16, 4, 1, boardId);
}

//______________________________________________________________________________
void Mapping::buildDETypeLarge(int rpcType, std::array<int, 7> boards)
{
  /// Builds segmentation for RPCs with largest pitch
  for (int icolumn = 0; icolumn < 6; ++icolumn) {
    setupSegmentation(rpcType, icolumn, 8, 4, 1, boards[icolumn]);
  }
  setupSegmentationLastColumn(rpcType, boards[6]);
}

//______________________________________________________________________________
void Mapping::buildDETypeMedium(int rpcType, std::array<int, 7> boards, bool largeNonBend)
{
  /// Builds segmentation for RPCs with medium pitch
  int stripPitchNBP = largeNonBend ? 4 : 2;
  int nStripsNBP = 32 / stripPitchNBP;
  for (int icolumn = 0; icolumn < 5; ++icolumn) {
    setupSegmentation(rpcType, icolumn, nStripsNBP, stripPitchNBP, 2, boards[icolumn]);
  }
  setupSegmentation(rpcType, 5, 8, 4, 2, boards[5]);
  setupSegmentationLastColumn(rpcType, boards[6]);
}

//______________________________________________________________________________
void Mapping::buildDETypeCut(int rpcType, std::array<int, 7> boards, bool isBelowBeamPipe)
{
  /// Builds segmentation for RPCs with cut
  setupSegmentation(rpcType, 0, 16, 2, 3, boards[0], isBelowBeamPipe);
  for (int icolumn = 1; icolumn < 6; ++icolumn) {
    int nBoardsBP = (icolumn < 3) ? 4 : 2;
    int stripPitchNBP = (icolumn < 5) ? 2 : 4;
    setupSegmentation(rpcType, icolumn, 32 / stripPitchNBP, stripPitchNBP, nBoardsBP, boards[icolumn]);
  }
  setupSegmentationLastColumn(rpcType, boards[6]);
}

//______________________________________________________________________________
void Mapping::buildDETypeShort(int rpcType, std::array<int, 7> boards)
{
  /// Builds segmentation for RPCs short
  setupSegmentation(rpcType, 0, 0, 0, 0, boards[0]);
  setupSegmentation(rpcType, 1, 8, 2, 4, boards[1]);
  for (int icolumn = 2; icolumn < 6; ++icolumn) {
    int nBoardsBP = (icolumn < 3) ? 4 : 2;
    int stripPitchNBP = (icolumn < 5) ? 2 : 4;
    setupSegmentation(rpcType, icolumn, 32 / stripPitchNBP, stripPitchNBP, nBoardsBP, boards[icolumn]);
  }
  setupSegmentationLastColumn(rpcType, boards[6]);
}

//______________________________________________________________________________
double Mapping::getStripSize(int strip, int cathode, int column, int deId) const
{
  /// Gets the strip size
  /// @param strip The strip number
  /// @param cathode Bending plane (0) or Non-Bending plane (1)
  /// @param column The column id in the detection element
  /// @param deId The detection element ID
  int rpcType = getRPCType(deId);
  int ichamber = Constants::getChamber(deId);
  int stripPitch = (cathode == 0) ? mDetectionElements[rpcType].columns[column].stripPitchBP
                                  : mDetectionElements[rpcType].columns[column].stripPitchNBP;
  if (cathode == 0) {
    strip = 0;
  }

  return getStripSize(ichamber, stripPitch, strip);
}

//______________________________________________________________________________
int Mapping::getNStripsNBP(int column, int deId) const
{
  /// Gets the number of strips in the NBP
  /// @param column The column id in the detection element
  /// @param deId The detection element ID
  int rpcType = getRPCType(deId);
  return mDetectionElements[rpcType].columns[column].nStripsNBP;
}

//______________________________________________________________________________
int Mapping::getFirstColumn(int deId) const
{
  /// Gets the first column in the DE
  /// @param deId The detection element ID
  int rpcType = getRPCType(deId);
  return (rpcType == 4) ? 1 : 0;
}

//______________________________________________________________________________
int Mapping::getFirstBoardBP(int column, int deId) const
{
  /// Gets the line number of the first board in the column
  /// @param column The column id in the detection element
  /// @param deId The detection element ID
  int rpcType = getRPCType(deId);
  return ((mDetectionElements[rpcType].columns[column].boardsBP[0] == 117) ? 1 : 0);
}

//______________________________________________________________________________
int Mapping::getLastBoardBP(int column, int deId) const
{
  /// Gets the number of boards in the column
  /// @param column The column id in the detection element
  /// @param deId The detection element ID
  int rpcType = getRPCType(deId);
  int lastBoard = mDetectionElements[rpcType].columns[column].boardsBP.size() - 1;
  if (column == 0 && rpcType == 3) {
    --lastBoard;
  }
  return lastBoard;
}

//______________________________________________________________________________
int Mapping::getBoardId(int line, int column, int deId, bool warn) const
{
  /// Gets the ID of boards in column
  /// @param line The local board line in the column
  /// @param column The column id in the detection element
  /// @param deId The detection element ID
  /// @param warn Set to false to avoid printing an error message in case the strip is not found (default: true)
  int rpcType = getRPCType(deId);
  if (line < mDetectionElements[rpcType].columns[column].boardsBP.size()) {
    int boardIdx = mDetectionElements[rpcType].columns[column].boardsBP[line];
    if (boardIdx < 117) {
      int boardId = boardIdx + 1;
      if (deId >= 36) {
        boardId += 117;
      }
      return boardId;
    }
  }
  if (warn) {
    std::cerr << "Cannot find local board in DE " << deId << "  column " << column << "  line " << line << std::endl;
  }
  return 0;
}

//______________________________________________________________________________
std::vector<Mapping::MpStripIndex> Mapping::getNeighbours(const Mapping::MpStripIndex& stripIndex, int cathode,
                                                          int deId) const
{
  /// Gets the list of neighbour strips
  /// @param stripIndex The indexes to identify the strip
  /// @param cathode Bending plane (0) or Non-Bending plane (1)
  /// @param deId The detection element ID

  int rpcType = getRPCType(deId);
  return (cathode == 0) ? getNeighboursBP(stripIndex, rpcType) : getNeighboursNBP(stripIndex, rpcType);
}

//______________________________________________________________________________
std::vector<Mapping::MpStripIndex> Mapping::getNeighboursNBP(const Mapping::MpStripIndex& stripIndex, int rpcType) const
{
  /// Gets the neighbours in the Non-Bending plane
  std::vector<MpStripIndex> neighbours;
  MpStripIndex neigh;
  for (int delta = -1; delta <= 1; delta += 2) {
    neigh.column = stripIndex.column;
    neigh.line = stripIndex.line;
    neigh.strip = stripIndex.strip + delta;
    if (neigh.strip < 0) {
      if (stripIndex.column == 0 || (stripIndex.column == 1 && rpcType == 4)) {
        continue;
      }
      --neigh.column;
      if (!isValidLine(neigh.line, neigh.column, rpcType)) {
        // This is useful for cut RPCs
        continue;
      }
      neigh.strip = mDetectionElements[rpcType].columns[neigh.column].nStripsNBP - 1;
    } else if (neigh.strip >= mDetectionElements[rpcType].columns[stripIndex.column].nStripsNBP) {
      if (stripIndex.column == 6) {
        continue;
      }
      ++neigh.column;
      neigh.strip = 0;
    }
    neighbours.push_back(neigh);
  }
  return neighbours;
}

//______________________________________________________________________________
std::vector<Mapping::MpStripIndex> Mapping::getNeighboursBP(const Mapping::MpStripIndex& stripIndex, int rpcType) const
{
  /// Gets the neighbours in the Bending plane
  std::vector<MpStripIndex> neighbours;
  MpStripIndex neigh;
  // First search in the same column
  for (int delta = -1; delta <= 1; delta += 2) {
    neigh.column = stripIndex.column;
    neigh.line = stripIndex.line;
    neigh.strip = stripIndex.strip + delta;
    if (neigh.strip < 0) {
      --neigh.line;
      if (!isValidLine(neigh.line, neigh.column, rpcType)) {
        continue;
      }
      neigh.strip = 15;
    } else if (neigh.strip >= 16) {
      ++neigh.line;
      if (!isValidLine(neigh.line, neigh.column, rpcType)) {
        continue;
      }
      neigh.strip = 0;
    }
    neighbours.push_back(neigh);
  }

  // Then search in the neighbour columns
  int stripPitch = mDetectionElements[rpcType].columns[stripIndex.column].stripPitchBP;
  for (int delta = -1; delta <= 1; delta += 2) {
    neigh.column = stripIndex.column + delta;
    if (!isValidColumn(neigh.column, rpcType)) {
      continue;
    }
    int stripPitchNeigh = mDetectionElements[rpcType].columns[neigh.column].stripPitchBP;
    int absStripNeigh = (16 * stripIndex.line + stripIndex.strip) * stripPitch / stripPitchNeigh;
    neigh.line = absStripNeigh / 16;
    if (!isValidLine(neigh.line, neigh.column, rpcType)) {
      continue;
    }
    neigh.strip = absStripNeigh % 16;
    neighbours.push_back(neigh);
    if (stripPitch > stripPitchNeigh) {
      neigh.strip = (neigh.strip % 2 == 0) ? neigh.strip + 1 : neigh.strip - 1;
      neighbours.push_back(neigh);
    }
  }

  return neighbours;
}

//______________________________________________________________________________
Mapping::MpStripIndex Mapping::nextStrip(const MpStripIndex& stripIndex, int cathode, int deId, bool descending) const
{
  /// Gets the next strip in the non bending plane
  /// @param stripIndex The indexes to identify the strip
  /// @param cathode Bending plane (0) or Non-Bending plane (1)
  /// @param deId The detection element ID
  /// @param descending Move from inner to outer
  MpStripIndex neigh = stripIndex;
  int rpcType = getRPCType(deId);
  int step = (descending) ? -1 : 1;
  neigh.strip = stripIndex.strip + step;
  int nStrips = (cathode == 0) ? 16 : mDetectionElements[rpcType].columns[neigh.column].nStripsNBP;
  if (neigh.strip < 0 || neigh.strip >= nStrips) {
    bool isOk = false;
    if (cathode == 0) {
      neigh.line += step;
      isOk = isValidLine(neigh.line, neigh.column, rpcType);
    } else {
      neigh.column += step;
      isOk = isValidColumn(neigh.column, rpcType);
    }
    if (isOk) {
      if (neigh.strip < 0) {
        if (cathode == 1) {
          nStrips = mDetectionElements[rpcType].columns[neigh.column].nStripsNBP;
        }
        neigh.strip = nStrips - 1;
      } else {
        neigh.strip = 0;
      }
    } else {
      neigh.column = 100;
    }
  }
  return neigh;
}

//______________________________________________________________________________
bool Mapping::isValid(int deId, int column, int cathode, int line, int strip) const
{
  /// Checks if required element is valid
  /// @param deId The detection element ID
  /// @param column The column id in the detection element
  /// @param cathode Bending plane (0) or Non-Bending plane (1)
  /// @param line The local board line in the column
  /// @param strip The strip number
  if (deId < 0 || deId >= 72) {
    return false;
  }
  int rpcType = getRPCType(deId);
  if (!isValidColumn(column, rpcType)) {
    return false;
  }
  if (!isValidLine(line, column, rpcType)) {
    return false;
  }

  int nStrips = (cathode == 0) ? 16 : mDetectionElements[rpcType].columns[column].nStripsNBP;
  if (strip < 0 || strip >= nStrips) {
    return false;
  }

  return true;
}

//______________________________________________________________________________
bool Mapping::isValidColumn(int column, int rpcType) const
{
  /// Checks if column exists
  if (column < 0 || column > 6 || (rpcType == 4 && column == 0)) {
    return false;
  }

  return true;
}

//______________________________________________________________________________
bool Mapping::isValidLine(int line, int column, int rpcType) const
{
  /// Checks if board is valid
  auto& columnStruct{mDetectionElements[rpcType].columns[column]};

  if (line >= columnStruct.boardsBP.size() || columnStruct.boardsBP[line] == 117) {
    return false;
  }

  return true;
}

//______________________________________________________________________________
double Mapping::getStripSize(int chamber, int stripPitch, int strip) const
{
  /// Gets the strip size of the strip in the chamber
  if (strip > 7 && stripPitch == 4) {
    stripPitch = 2;
  }
  return Constants::getStripUnitPitchSize(chamber) * stripPitch;
}

//______________________________________________________________________________
double Mapping::getColumnLeftPosition(int column, int chamber, int rpcType) const
{
  /// Returns the left position of the column
  double xCenter = Constants::getRPCHalfLength(chamber, rpcType);
  double col = (double)column;
  if (rpcType == 4) {
    col += (column == 1) ? -1. : -1.5;
  }
  return Constants::getLocalBoardWidth(chamber) * col - xCenter;
}

//______________________________________________________________________________
double Mapping::getColumnBottomPosition(int column, int chamber, int rpcType) const
{
  /// Returns the bottom position of the column
  double nBoardsDown = (rpcType == 5 && column == 0) ? 1. : 2.;
  return -Constants::getLocalBoardHeight(chamber) * nBoardsDown;
}

//______________________________________________________________________________
double Mapping::getColumnHeight(int column, int chamber, int rpcType) const
{
  /// Gets the column height
  double sizeFactor = 1.;
  if (column == 0 && (rpcType == 3 || rpcType == 5)) {
    sizeFactor = 0.75;
  }
  return 4 * Constants::getLocalBoardHeight(chamber) * sizeFactor;
}

//______________________________________________________________________________
double Mapping::getColumnWidth(int column, int chamber, int rpcType) const
{
  /// Gets the column width
  double sizeFactor = 1.;
  if (column == 6) {
    sizeFactor = 1.5;
  } else if (column == 1 && rpcType == 4) {
    sizeFactor = 0.5;
  }
  return Constants::getLocalBoardWidth(chamber) * sizeFactor;
}

//______________________________________________________________________________
double Mapping::getStripLowEdge(int strip, int stripPitch, int line, int chamber) const
{
  /// Gets position of the low edge of the strip
  return (Constants::getStripUnitPitchSize(chamber) * stripPitch * (strip + 16 * line) -
          2. * Constants::getLocalBoardHeight(chamber));
}

//______________________________________________________________________________
double Mapping::getStripLeftEdge(int strip, int stripPitch, int column, int chamber, int rpcType) const
{
  /// Gets the position of the left edge of the strip
  if (column == 6 && strip > 7) {
    column = 7;
    strip = strip - 8;
    stripPitch = 2;
  }
  return getStripSize(chamber, stripPitch) * strip + getColumnLeftPosition(column, chamber, rpcType);
}

//______________________________________________________________________________
MpArea Mapping::stripByLocation(int strip, int cathode, int line, int column, int deId, bool warn) const
{
  /// Gets the strip area from its location
  /// @param strip The strip number
  /// @param cathode Bending plane (0) or Non-Bending plane (1)
  /// @param line The local board line in the column
  /// @param column The column id in the detection element
  /// @param deId The detection element ID
  /// @param warn Set to false to avoid printing an error message in case the strip is not found (default: true)
  int deType = getRPCType(deId);
  int chamber = Constants::getChamber(deId);
  assert(strip < 16);
  auto& columnStruct{mDetectionElements[deType].columns[column]};

  double x1 = 0., y1 = 0., x2 = 0., y2 = 0.;

  if (cathode == 0) {
    x1 = getColumnLeftPosition(column, chamber, deType);
    x2 = x1 + getColumnWidth(column, chamber, deType);
    // y1 = ((boardIndex.line - column.boardsBP.size() / 2) * 8 + strip) * stripSizeBP;
    y1 = getStripLowEdge(strip, columnStruct.stripPitchBP, line, chamber);
    y2 = y1 + getStripSize(chamber, columnStruct.stripPitchBP);
  } else if (strip < columnStruct.nStripsNBP) {
    // double yDim = 0.5 * column.boardsBP.size() * 16. * getStripSize(chamber, column.stripPitchBP);
    y1 = getColumnBottomPosition(column, chamber, deType);
    // y2 = y1 + 16 * getStripSize(chamber, column.stripPitchBP) * column.boardsBP.size();
    y2 = y1 + getColumnHeight(column, chamber, deType);
    // double stripSizeNBP = getStripSize(chamber, column.stripPitchNBP,strip);
    // x1 = column.xCenter + (strip - column.nStripsNBP / 2) * pitchNBP;
    x1 = getStripLeftEdge(strip, columnStruct.stripPitchNBP, column, chamber, deType);
    x2 = x1 + getStripSize(chamber, columnStruct.stripPitchNBP, strip);
  } else if (warn) {
    std::cout << "Warning: no non-bending strip " << strip << " in column " << column << " of DE " << deId << std::endl;
  }

  return MpArea(x1, y1, x2, y2);
}

//______________________________________________________________________________
MpArea Mapping::stripByLocationInBoard(int strip, int cathode, int boardId, int chamber, bool warn) const
{
  /// Gets the strip area from its location
  /// @param strip The strip number
  /// @param cathode Bending plane (0) or Non-Bending plane (1)
  /// @param boardId The id of the local board
  /// @param chamber The chamber number (0-3)
  /// @param warn Set to false to avoid printing an error message in case the strip is not found (default: true)
  assert(boardId <= 234);
  auto& boardIndex{mBoardIndexes[(boardId - 1) % 117]};
  int deId = boardIndex.deType + 9 * chamber;
  if (boardId > 117) {
    deId += 36;
  }
  return stripByLocation(strip, cathode, boardIndex.line, boardIndex.column, deId, warn);
}

//______________________________________________________________________________
int Mapping::getColumn(double xPos, int chamber, int rpcType) const
{
  /// Gets the column from x position
  double xShift = xPos + Constants::getRPCHalfLength(chamber, rpcType);
  int invalid = 7;
  if (xShift < 0) {
    return invalid;
  }
  double boardUnitWidth = Constants::getLocalBoardWidth(chamber);
  if (rpcType == 4) {
    xShift += 1.5 * boardUnitWidth;
  }
  double column = xShift / boardUnitWidth;
  if (column > 7.5) {
    return invalid;
  } else if (column > 6) {
    return 6;
  }

  return (int)column;
}

//______________________________________________________________________________
int Mapping::getLine(double yPos, const MpColumn& column, int chamber) const
{
  /// Gets the board from y position
  unsigned long int nBoards = column.boardsBP.size();
  int invalid = 4;
  if (nBoards == 0) {
    return invalid;
  }
  double boardUnitHeight = Constants::getLocalBoardHeight(chamber);
  double yShift = yPos + 2. * boardUnitHeight;
  if (yShift < 0) {
    return invalid;
  }
  double boardHeight = boardUnitHeight * 4 / nBoards;
  int iline = (int)(yShift / boardHeight);
  if (iline >= nBoards) {
    return invalid;
  }
  if (column.boardsBP[iline] == 117) {
    return invalid;
  }
  return iline;
}

//______________________________________________________________________________
Mapping::MpStripIndex Mapping::stripByPosition(double xPos, double yPos, int cathode, int deId, bool warn) const
{
  /// Gets the strip containing the coordinate xPos, yPos
  /// @param xPos x coordinate in the detection element
  /// @param yPos y coordinate in the detection element
  /// @param cathode Bending plane (0) or Non-Bending plane (1)
  /// @param deId The detection element ID
  /// @param warn Set to false to avoid printing an error message in case the strip is not found (default: true)
  assert(deId < 72);
  int rpcType = getRPCType(deId);
  int chamber = Constants::getChamber(deId);
  MpStripIndex stripIndex;
  stripIndex.column = getColumn(xPos, chamber, rpcType);
  stripIndex.line = 4;
  stripIndex.strip = 16;

  if (stripIndex.column >= 7) {
    if (warn) {
      std::cout << "Warning: xPos " << xPos << " not inside RPC " << deId << std::endl;
    }
    return stripIndex;
  }

  const auto& column{mDetectionElements[rpcType].columns[stripIndex.column]};
  stripIndex.line = getLine(yPos, column, chamber);
  if (stripIndex.line >= 4) {
    if (warn) {
      std::cout << "Warning: yPos " << yPos << " not inside RPC " << deId << std::endl;
    }
    return stripIndex;
  }

  if (cathode == 0) {
    double yBoardLow = Constants::getLocalBoardHeight(chamber) * (stripIndex.line * 4 / column.boardsBP.size() - 2.);
    double stripSize = getStripSize(chamber, column.stripPitchBP);
    stripIndex.strip = (int)((yPos - yBoardLow) / stripSize);
  } else {
    double xBoardLeft = getColumnLeftPosition(stripIndex.column, chamber, rpcType);
    double stripSize = getStripSize(chamber, column.stripPitchNBP);
    double diff = xPos - xBoardLeft;
    int stripOffset = 0;
    if (stripIndex.column == 6) {
      double offset = 8. * stripSize;
      if (diff > offset) {
        diff -= offset;
        stripSize = stripSize / 2.;
        stripOffset = 8;
      }
    }
    stripIndex.strip = (int)(diff / stripSize) + stripOffset;
  }
  return stripIndex;
}

} // namespace mid
} // namespace o2
