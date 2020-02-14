#ifndef SILOHELPERS_HPP
#define SILOHELPERS_HPP

#include <silo.h>

#include <memory>
#include <type_traits>

inline void SiloFree(DBfile *const obj) { DBClose(obj); }
inline void SiloFree(DBmultimesh *const obj) { DBFreeMultimesh(obj); }
inline void SiloFree(DBmultivar *const obj) { DBFreeMultivar(obj); }
inline void SiloFree(DBoptlist *const obj) { DBFreeOptlist(obj); }
inline void SiloFree(DBquadmesh *const obj) { DBFreeQuadmesh(obj); }
inline void SiloFree(DBquadvar *const obj) { DBFreeQuadvar(obj); }
inline void SiloFree(DBtoc *const obj) {}
inline void SiloFree(char *const obj) { free(obj); }

template <typename T> using Silo = std::shared_ptr<T>;
template <typename T> Silo<T> MakeSilo(T *const obj) {
  return std::shared_ptr<T>(
      obj, static_cast<void (*)(typename std::remove_cv<T>::type *)>(SiloFree));
}

#endif // #ifndef SILOHELPERS_HPP
