#pragma once
#include "Units.hpp"
#include "MMapvector.hpp"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
#include <variant>
// -------------------------------------------------------------------------------------
namespace cengine {
// -------------------------------------------------------------------------------------
class Column {
public:
   const ColumnType type;
   const string name;
   std::variant<Vector<INTEGER>, Vector<DOUBLE>, Vector<str>> data;
   Vector<BITMAP> bitmap;

   Column(const ColumnType type, const string name, const string data_path, const string bitmap_path);
   const Vector<INTEGER> &integers() const;
   const Vector<DOUBLE> &doubles() const;
   const Vector<str> &strings() const;
   const Vector<BITMAP> &bitmaps() const;
   SIZE sizeInBytes() const;
};
// -------------------------------------------------------------------------------------
}
// -------------------------------------------------------------------------------------
