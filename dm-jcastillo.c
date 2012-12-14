/*
 * Copyright (C) 2012 Jose Castllo <jcastillo@redhat.com>
 *
 * This file is released under the GPL.
 */

#include <linux/device-mapper.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>

#define DM_MSG_PREFIX "jcastillo"

/*
 * Here we build a mapping that returns only Js
 */

static int jcastillo_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	if (argc != 0) {
		ti->error = "No arguments duh";
		return -EINVAL;
	}

	/*
 	 *	Lets dropdiscards the zero target way
 	 */

	ti->num_discard_requests = 1;

	return 0; 	
}


/* 
 * Return J only on reads
 */

static int jcastillo_map(struct dm_target *ti, struct bio *bio, 
				union map_info *map_context)
{
	unsigned long flags;
        struct bio_vec *bv;
        int i;

	switch(bio_rw(bio)) {
	case READ: 
	/* Here I'll modify the bio if possible
 	 * to add "J"
 	 */

        bio_for_each_segment(bv, bio, i) {
                char *data = bvec_kmap_irq(bv, &flags);
                memset(data, 'J', bv->bv_len);
                flush_dcache_page(bv->bv_page);
                bvec_kunmap_irq(data, &flags);
		}

		break;
	case READA:
	/* Same as in the zero target */
		return -EIO;
	case WRITE:
	/* You shall not pass! */
		break;
	}

	bio_endio(bio,0);
	/* accepted bio, don't make new request */
	return DM_MAPIO_SUBMITTED;
//	return 0;
}

static struct target_type jcastillo_target = {
	.name = "jcastillo",
	.version = {0,0,1},
	.module = THIS_MODULE,
	.ctr	= jcastillo_ctr,
	.map	= jcastillo_map,
};

static int __init dm_jcastillo_init(void)
{
	int r = dm_register_target(&jcastillo_target);

	if (r < 0)
                DMERR("register failed %d", r);

        return r;
}

static void __exit dm_jcastillo_exit(void)
{
	dm_unregister_target(&jcastillo_target);
}

module_init(dm_jcastillo_init);
module_exit(dm_jcastillo_exit);

MODULE_AUTHOR("Jose Castillo <jcastillo@redhat.com>");
MODULE_DESCRIPTION(DM_NAME " test module that returns J");
MODULE_LICENSE("GPL");
