/**
 * Created on: 6/4/16
 *     Author: xing
 */

#pragma once

#include <cstring>
#include <mutex>
#include <functional>

#include "utils_cpp/module.h"

namespace utils
{
/**
 * Buffer
 */
class Buffer
{
 public:
  Buffer() = delete;

  inline void
  init(size_t initial_size, size_t initial_len) noexcept
  {
    size = initial_size;
    len = initial_len;
  }

  Ret
  write(const uint8_t *src, size_t length) noexcept;

  Ret
  read(uint8_t *dst, size_t &length) const noexcept;

  uint8_t *
  get_data() noexcept
  {
    return data;
  }

 private:
  UTILS_READER(size_t, size);
  UTILS_READER(size_t, len);

  static constexpr size_t prefix_len{sizeof(size_t) + sizeof(size_t)};

#ifdef UNIX
  uint8_t pad[UTILS_ROUND(prefix_len, sizeof(void *)) - prefix_len];
#endif
  uint8_t data[sizeof(void *)];
};

typedef std::function<void(Buffer *)> buf_callback;
typedef std::unique_ptr<Buffer, buf_callback> unique_buf;
typedef std::shared_ptr<Buffer> shared_buf;

/**
 * Buffer Pool
 * @note Not thread safe. Try CBufferPool in multi-thread circumstances
 */
class BufferPool : public Module
{
 public:
  BufferPool(size_t buf_count, size_t buf_size)
      : BufferPool(buf_count, buf_size, "")
  {
  }

  BufferPool(size_t buf_count, size_t buf_size, const std::string &name);

  ~BufferPool() override
  {
    put_stat();
    delete[] mem;
  }

  inline unique_buf
  alloc_unique() noexcept
  {
    return unique_buf{alloc(), del};
  }

  inline shared_buf
  alloc_shared() noexcept
  {
    return shared_buf{alloc(), del};
  }

  std::string
  to_s() const noexcept override
  {
    return get_name() + stat.to_s();
  }

 private:
  UTILS_DISALLOW_COPY_AND_ASSIGN(BufferPool)

  struct NodeHead {
    NodeHead() = default;

    explicit NodeHead(void *ptr) : next{(NodeHead *)ptr} {}

    NodeHead *next{};
  };

  typedef NodeHead *nodeptr;

  Buffer *
  alloc() noexcept;

  buf_callback del{[this](Buffer *b) {
    unless(b) { return; }
    ((nodeptr)b)->next = free_mem;
    free_mem = (nodeptr)b;
  }};

  struct Stat {
    inline std::string
    to_s() const noexcept
    {
      UTILS_STR_S(32)
      UTILS_STR_ATTR(total)
      UTILS_STR_ATTR(succ)
      return s;
    }

    uint64_t total{};
    uint64_t succ{};
  } stat{};

  UTILS_READER(size_t, buf_count);
  UTILS_READER(size_t, buf_size);
  uint8_t *mem;
  nodeptr free_mem;
};

/**
 * Concurrent Buffer Pool
 * @note This Pool is thread safe.
 * 	For run-time efficiency, we create this stand alone concurrent buffer
 * pool other than add cocurrent feature into existing Buffer Pool
 */
class CBufferPool : public Module
{
 public:
  CBufferPool(size_t buf_count, size_t buf_size)
      : CBufferPool(buf_count, buf_size, "")
  {
  }

  CBufferPool(size_t buf_count, size_t buf_size, const std::string &name);

  ~CBufferPool() override
  {
    put_stat();
    delete[] mem;
  }

  inline unique_buf
  alloc_unique() noexcept
  {
    return unique_buf{alloc(), del};
  }

  inline shared_buf
  alloc_shared() noexcept
  {
    return shared_buf{alloc(), del};
  }

  std::string
  to_s() const noexcept override
  {
    return get_name() + stat.to_s();
  }

 private:
  UTILS_DISALLOW_COPY_AND_ASSIGN(CBufferPool)

  struct NodeHead {
    NodeHead() = default;

    explicit NodeHead(void *ptr) : next{(NodeHead *)ptr} {}

    NodeHead *next{};
  };

  typedef NodeHead *nodeptr;

  Buffer *
  alloc() noexcept;

  buf_callback del{[this](Buffer *b) {
    unless(b) { return; }
    {
      std::lock_guard<std::mutex> bar{mut};
      ((nodeptr)b)->next = free_mem;
      free_mem = (nodeptr)b;
    }
  }};

  struct Stat {
    inline std::string
    to_s() const noexcept
    {
      UTILS_STR_S(32)
      UTILS_STR_ATTR(total)
      UTILS_STR_ATTR(succ)
      return s;
    }

    uint64_t total{};
    uint64_t succ{};
  } stat{};

  UTILS_READER(size_t, buf_count);
  UTILS_READER(size_t, buf_size);
  uint8_t *mem;
  nodeptr free_mem;
  std::mutex mut{};
};

}  // namespace utils
