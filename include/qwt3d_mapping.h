#ifndef qwt3d_mapping_h__2004_03_05_13_51_begin_guarded_code
#define qwt3d_mapping_h__2004_03_05_13_51_begin_guarded_code

#include "qwt3d_global.h"
#include "qwt3d_types.h"
#include <qstring.h>

namespace Qwt3D {

//! Abstract base class for general mappings
/**

*/
class QWT3D_EXPORT Mapping
{
public:
    virtual ~Mapping() {}                                //!< Destructor.
    virtual QString name() const { return QString(""); } //!< Descriptive String.
};

} // namespace Qwt3D

#endif /* include guarded */
