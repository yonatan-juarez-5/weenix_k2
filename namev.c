/******************************************************************************/
/* Important Fall 2024 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5fd1e93dbf35cbffa3aef28f8c01d8cf2ffc51ef62b26a       */
/*         f9bda5a68e5ed8c972b17bab0f42e24b19daa7bd408305b1f7bd6c7208c1       */
/*         0e36230e913039b3046dd5fd0ba706a624d33dbaa4d6aab02c82fe09f561       */
/*         01b0fd977b0051f0b0ce0c69f7db857b1b5e007be2db6d42894bf93de848       */
/*         806d9152bd5715e9                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "kernel.h"
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
        /* the "dir" argument must be non-NULL */
        KASSERT(NULL != dir); 
        dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
        /* the "name" argument must be non-NULL */
        KASSERT(NULL != name); 
        dbg(DBG_PRINT, "(GRADING2A 2.a\n)");
        /* the "result" argument must be non-NULL */
        KASSERT(NULL != result); 
        dbg(DBG_PRINT, "(GRADING2A 2.a\n)");

        if (dir->vn_ops->lookup == NULL){
                dbg(DBG_PRINT, "(GRADING2A 2.a\n)");
                return -ENOTDIR; /* Not a directory */
        }

        dbg(DBG_PRINT, "(GRADING2A 2.a\n)");
        int res = 0;
        res = dir->vn_ops->lookup(dir, name, len, result);
        dbg(DBG_PRINT, "(GRADING2A 2.a\n)");
        return res;
        // NOT_YET_IMPLEMENTED("VFS: lookup");
        // return 0;
}


/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int
dir_namev(const char *pathname, size_t *namelen, const char **name,
          vnode_t *base, vnode_t **res_vnode)
{
        /* dir_vnode is the directory vnode where you start the pathname resolution process */

        /* the "pathname" argument must be non-NULL */
        KASSERT(NULL != pathname); 
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
        
        /* the "namelen" argument must be non-NULL */
        KASSERT(NULL != namelen); 
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
        
        /* the "name" argument must be non-NULL */
        KASSERT(NULL != name);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
        
        /* the "res_vnode" argument must be non-NULL */
        KASSERT(NULL != res_vnode); 
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        if (*pathname == '\0'){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EINVAL; /* Invalid argument */
        }

        vnode_t *prev = NULL, *base_vnode = NULL;
        int i = 0, offset = 0, len = 0, ret_code = 0;


        if (pathname[0] == '/'){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                base_vnode = vfs_root_vn;
        }
        else if (base != NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                base_vnode = base;
        }
        else{
                dbg(DBG_PRINT, "(GRADING2B)\n");
                base_vnode = curproc->p_cwd;
        }

        vref(base_vnode);

        while(pathname[i] == '/'){
            dbg(DBG_PRINT, "(GRADING2B)\n");
                i++;
        }

        do{
                len = 0;
                offset = i;
                if (pathname[i] == '\0' && prev == NULL){
                        *namelen = 1;
                        *name = ".";
                        *res_vnode = base_vnode;
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return 0;
                }
                if (prev != NULL){
                        vput(prev);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                }
                prev = base_vnode;
                while(pathname[i] != '/' && pathname[i] != '\0'){
                        i++;
                        len++;
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                }
                // dbg(DBG_PRINT, "(GRADING2B)\n");

                if (len > NAME_LEN){
                        vput(prev);
                        return -ENAMETOOLONG;
                } 

                ret_code = lookup(prev, pathname+offset, len, &base_vnode);
                /* pathname resolution must start with a valid directory */
                KASSERT(NULL != base_vnode); 
                dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
                if ((ret_code == -ENOTDIR) || (ret_code < 0 && ret_code != -ENOENT)){
                        vput(prev);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return ret_code;
                }

                while(pathname[i] == '/'){
                    dbg(DBG_PRINT, "(GRADING2B)\n");
                    i++;
                }

        }while(pathname[i] != '\0' && ret_code >= 0);


        if (pathname[i-1] == '/'){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                if (!S_ISDIR(base_vnode->vn_mode)){
                        vput(prev);
                        vput(base_vnode);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return -ENOTDIR;
                }
        }

        if (pathname[i] != '\0'){
            dbg(DBG_PRINT, "(GRADING2B)\n");
            vput(prev);
            return -ENOENT;
        }
        else if(ret_code == 0){
            dbg(DBG_PRINT, "(GRADING2B)\n");
            vput(base_vnode);
        }
        dbg(DBG_PRINT, "(GRADING2B)\n");
        *name = pathname + offset;
        *namelen = len;
        *res_vnode = prev;
        
        // // NOT_YET_IMPLEMENTED("VFS: dir_namev");
        return 0;
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified and the file does
 * not exist, call create() in the parent directory vnode. However, if the
 * parent directory itself does not exist, this function should fail - in all
 * cases, no files or directories other than the one at the very end of the path
 * should be created.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
        size_t len = 0;
        const char *name = NULL;
        vnode_t *dir_vnode = NULL;
        int retCode = dir_namev(pathname, &len, &name, base, &dir_vnode);

        if (retCode < 0){
                dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                return retCode;
        }
        retCode = lookup(dir_vnode, name, len, res_vnode);
        if (retCode < 0){
                /* please use TWO consecutive "conforming dbg() calls" for this 
                since this function is not executed if you just start and stop weenix */

                /* to create a file, need to make sure that the directory 
                // vnode supports the create operation */

                /* No such file or directory */
                if ((flag & O_CREAT) && retCode == -ENOENT){
                        retCode = dir_vnode->vn_ops->create(dir_vnode, name, len, res_vnode);
                        vput(dir_vnode);
                        dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return retCode;
                }
                else{
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        vput(dir_vnode);
                        return retCode;
                }
        }
        else{
            dbg(DBG_PRINT, "(GRADING2B)\n");
            vput(dir_vnode);    
            return 0;
        }
        // NOT_YET_IMPLEMENTED("VFS: open_namev");
        // return 0;
}

#ifdef __GETCWD__
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
        return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

        return -ENOENT;
}
#endif /* __GETCWD__ */
