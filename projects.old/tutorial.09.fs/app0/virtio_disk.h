#ifndef VIRTIO_DISK_H_
#define VIRTIO_DISK_H_

#include "buf.h"
#include "types.h"

int virtio_disk_status(void);
void virtio_disk_init(void);
void virtio_disk_rw(struct buf *b, int write);

#endif // VIRTIO_DISK_H_
