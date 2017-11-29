#ifndef MARISA_ALPHA_BASE_H_
#define MARISA_ALPHA_BASE_H_

// Visual C++ does not provide stdint.h.
#ifndef _MSC_VER
#include <stdint.h>
#endif  // _MSC_VER

#ifdef __cplusplus
#include <cstddef>
#include <new>
#else  // __cplusplus
#include <stddef.h>
#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#ifdef _MSC_VER
typedef unsigned __int8  marisa_alpha_uint8;
typedef unsigned __int16 marisa_alpha_uint16;
typedef unsigned __int32 marisa_alpha_uint32;
typedef unsigned __int64 marisa_alpha_uint64;
#else  // _MSC_VER
typedef uint8_t  marisa_alpha_uint8;
typedef uint16_t marisa_alpha_uint16;
typedef uint32_t marisa_alpha_uint32;
typedef uint64_t marisa_alpha_uint64;
#endif  // _MSC_VER

#define MARISA_ALPHA_UINT8_MAX  ((marisa_alpha_uint8)-1)
#define MARISA_ALPHA_UINT16_MAX ((marisa_alpha_uint16)-1)
#define MARISA_ALPHA_UINT32_MAX ((marisa_alpha_uint32)-1)
#define MARISA_ALPHA_UINT64_MAX ((marisa_alpha_uint64)-1)
#define MARISA_ALPHA_SIZE_MAX   ((size_t)-1)

#define MARISA_ALPHA_ZERO_TERMINATED MARISA_ALPHA_UINT32_MAX
#define MARISA_ALPHA_NOT_FOUND       MARISA_ALPHA_UINT32_MAX
#define MARISA_ALPHA_MISMATCH        MARISA_ALPHA_UINT32_MAX

#define MARISA_ALPHA_MAX_LENGTH     (MARISA_ALPHA_UINT32_MAX - 1)
#define MARISA_ALPHA_MAX_NUM_KEYS   (MARISA_ALPHA_UINT32_MAX - 1)

// marisa_alpha_status provides a list of error codes. Most of functions in
// libmarisa throw or return an error code.
typedef enum marisa_alpha_status_ {
  // MARISA_ALPHA_OK means that a requested operation has succeeded.
  MARISA_ALPHA_OK               = 0,

  // MARISA_ALPHA_HANDLE_ERROR means that a given handle is invalid.
  MARISA_ALPHA_HANDLE_ERROR     = 1,

  // MARISA_ALPHA_STATE_ERROR means that an object is not ready for a requested
  // operation. For example, an operation to modify a fixed container throws
  // an exception with this error code.
  MARISA_ALPHA_STATE_ERROR      = 2,

  // MARISA_ALPHA_PARAM_ERROR means that a given argument is invalid. For
  // example, some functions throw an exception with this error code when an
  // out-of-range value or a NULL pointer is given.
  MARISA_ALPHA_PARAM_ERROR      = 3,

  // MARISA_ALPHA_SIZE_ERROR means that a size exceeds its limit. This error
  // code is used when a building dictionary is too large or std::length_error
  // is catched.
  MARISA_ALPHA_SIZE_ERROR       = 4,

  // MARISA_ALPHA_MEMORY_ERROR means that a memory allocation has failed.
  MARISA_ALPHA_MEMORY_ERROR     = 5,

  // MARISA_ALPHA_IO_ERROR means that an I/O failure.
  MARISA_ALPHA_IO_ERROR         = 6,

  // MARISA_ALPHA_UNEXPECTED_ERROR means that an unexpected error has occurred.
  MARISA_ALPHA_UNEXPECTED_ERROR = 7
} marisa_alpha_status;

// marisa_alpha_strerror() returns a name of an error code.
const char *marisa_alpha_strerror(marisa_alpha_status status);

// Flags and masks for dictionary settings are defined as follows. Please note
// that unspecified value/flags will be replaced with default value/flags.
typedef enum marisa_alpha_flags_ {
  // A dictionary consinsts of 3 tries in default. If you want to change the
  // number of tries, please give it with other flags.
  MARISA_ALPHA_MIN_NUM_TRIES     = 0x00001,
  MARISA_ALPHA_MAX_NUM_TRIES     = 0x000FF,
  MARISA_ALPHA_DEFAULT_NUM_TRIES = 0x00003,

  // MARISA_ALPHA_PATRICIA_TRIE is usually a better choice. MARISA_ALPHA_PREFIX_TRIE is
  // provided for comparing prefix/patricia tries.
  MARISA_ALPHA_PATRICIA_TRIE     = 0x00100,
  MARISA_ALPHA_PREFIX_TRIE       = 0x00200,
  MARISA_ALPHA_DEFAULT_TRIE      = MARISA_ALPHA_PATRICIA_TRIE,

  // There are 3 kinds of TAIL implementations.
  // - MARISA_ALPHA_WITHOUT_TAIL:
  //   builds a dictionary without a TAIL. Its last trie has only 1-byte
  //   labels.
  // - MARISA_ALPHA_BINARY_TAIL:
  //   builds a dictionary with a binary-mode TAIL. Its last labels are stored
  //   as binary data.
  // - MARISA_ALPHA_TEXT_TAIL:
  //   builds a dictionary with a text-mode TAIL if its last labels do not
  //   contain NULL characters. The last labels are stored as zero-terminated
  //   string. Otherwise, a dictionary is built with a binary-mode TAIL.
  MARISA_ALPHA_WITHOUT_TAIL      = 0x01000,
  MARISA_ALPHA_BINARY_TAIL       = 0x02000,
  MARISA_ALPHA_TEXT_TAIL         = 0x04000,
  MARISA_ALPHA_DEFAULT_TAIL      = MARISA_ALPHA_TEXT_TAIL,

  // libmarisa arranges nodes in ascending order of their labels
  // (MARISA_ALPHA_LABEL_ORDER) or in descending order of their weights
  // (MARISA_ALPHA_WEIGHT_ORDER). MARISA_ALPHA_WEIGHT_ORDER is generally a
  // better choice because it enables faster lookups, but
  // MARISA_ALPHA_LABEL_ORDER is still useful if an application needs to
  // predict keys in label order.
  MARISA_ALPHA_LABEL_ORDER       = 0x10000,
  MARISA_ALPHA_WEIGHT_ORDER      = 0x20000,
  MARISA_ALPHA_DEFAULT_ORDER     = MARISA_ALPHA_WEIGHT_ORDER,

  // The default settings. 0 is equivalent to MARISA_ALPHA_DEFAULT_FLAGS.
  MARISA_ALPHA_DEFAULT_FLAGS     = MARISA_ALPHA_DEFAULT_NUM_TRIES
      | MARISA_ALPHA_DEFAULT_TRIE | MARISA_ALPHA_DEFAULT_TAIL | MARISA_ALPHA_DEFAULT_ORDER,

  MARISA_ALPHA_NUM_TRIES_MASK    = 0x000FF,
  MARISA_ALPHA_TRIE_MASK         = 0x00F00,
  MARISA_ALPHA_TAIL_MASK         = 0x0F000,
  MARISA_ALPHA_ORDER_MASK        = 0xF0000,
  MARISA_ALPHA_FLAGS_MASK        = 0xFFFFF
} marisa_alpha_flags;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#ifdef __cplusplus
namespace marisa_alpha {

typedef ::marisa_alpha_uint8  UInt8;
typedef ::marisa_alpha_uint16 UInt16;
typedef ::marisa_alpha_uint32 UInt32;
typedef ::marisa_alpha_uint64 UInt64;

typedef ::marisa_alpha_status Status;

// An exception object stores a filename, a line number and an error code.
class Exception {
 public:
  Exception(const char *filename, int line, Status status)
      : filename_(filename), line_(line), status_(status) {}
  Exception(const Exception &ex)
      : filename_(ex.filename_), line_(ex.line_), status_(ex.status_) {}

  Exception &operator=(const Exception &rhs) {
    filename_ = rhs.filename_;
    line_ = rhs.line_;
    status_ = rhs.status_;
    return *this;
  }

  const char *filename() const {
    return filename_;
  }
  int line() const {
    return line_;
  }
  Status status() const {
    return status_;
  }

  // Same as std::exception, what() returns an error message.
  const char *what() const {
    return ::marisa_alpha_strerror(status_);
  }

 private:
  const char *filename_;
  int line_;
  Status status_;
};

// MARISA_ALPHA_THROW adds a filename and a line number to an exception.
#define MARISA_ALPHA_THROW(status) \
  (throw Exception(__FILE__, __LINE__, status))

// MARISA_ALPHA_THROW_IF throws an exception with `status' if `cond' is true.
#define MARISA_ALPHA_THROW_IF(cond, status) \
  (void)((!(cond)) || (MARISA_ALPHA_THROW(status), 0))

// MARISA_ALPHA_DEBUG_IF is used for debugging. For example,
// MARISA_ALPHA_DEBUG_IF is used to find out-of-range accesses in
// marisa::Vector, marisa::IntVector, etc.
#ifdef _DEBUG
#define MARISA_ALPHA_DEBUG_IF(cond, status) \
  MARISA_ALPHA_THROW_IF(cond, status)
#else
#define MARISA_ALPHA_DEBUG_IF(cond, status)
#endif

// To not include <algorithm> only for std::swap().
template <typename T>
void Swap(T *lhs, T *rhs) {
  MARISA_ALPHA_THROW_IF((lhs == NULL) || (rhs == NULL),
                        MARISA_ALPHA_PARAM_ERROR);
  T temp = *lhs;
  *lhs = *rhs;
  *rhs = temp;
}

}  // namespace marisa_alpha
#endif  // __cplusplus

#endif  // MARISA_ALPHA_BASE_H_
