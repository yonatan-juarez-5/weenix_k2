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

/*
 *  FILE: vfs_syscall.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Wed Apr  8 02:46:19 1998
 *  $Id: vfs_syscall.c,v 1.2 2018/05/27 03:57:26 cvsps Exp $
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"

/*
 * Syscalls for vfs. Refer to comments or man pages for implementation.
 * Do note that you don't need to set errno, you should just return the
 * negative error code.
 */

/* To read a file:
 *      o fget(fd)
 *      o call its virtual read vn_op
 *      o update f_pos
 *      o fput() it
 *      o return the number of bytes read, or an error
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for reading.
 *      o EISDIR
 *        fd refers to a directory.
 *
 * In all cases, be sure you do not leak file refcounts by returning before
 * you fput() a file that you fget()'ed.
 */
int
do_read(int fd, void *buf, size_t nbytes)
{
        if (fd < 0 || fd >= NFILES){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; /* Bad file number */
        }
        file_t *file = NULL;
        file = fget(fd);

        if(file == NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        
        // NO ERROR
        dbg(DBG_PRINT, "(GRADING2B)\n");
        if (file->f_mode & FMODE_READ){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                if(!(S_ISDIR(file->f_vnode->vn_mode)) && file->f_vnode->vn_ops->read != NULL){
                        unsigned int bytes = file->f_vnode->vn_ops->read(file->f_vnode,file->f_pos,buf,nbytes);
                        int seekResult = 0;
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        if(bytes>0){
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                seekResult = do_lseek(fd,bytes,SEEK_CUR);
                        }
                        else if (bytes == 0){
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                if(nbytes!=0){
                                        dbg(DBG_PRINT, "(GRADING2B)\n");
                                        seekResult = do_lseek(fd,0,SEEK_END);
                                }
                        }
                        fput(file);
                        if(seekResult<0){
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                return seekResult;
                        }
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return bytes;
                }
                else{
                        fput(file);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return -EISDIR;
                }
        }
        else{
                fput(file);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        // NOT_YET_IMPLEMENTED("VFS: do_read");
        // return -1;
}

/* Very similar to do_read.  Check f_mode to be sure the file is writable.  If
 * f_mode & FMODE_APPEND, do_lseek() to the end of the file, call the write
 * vn_op, and fput the file.  As always, be mindful of refcount leaks.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for writing.
 */
int
do_write(int fd, const void *buf, size_t nbytes)
{
        if (fd < 0 || fd >= NFILES){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; /* Bad file number */
        }
        file_t *file = NULL;
        file = fget(fd);

        if (file == NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; /* Bad file number */
        }

        // NO ERROR
        dbg(DBG_PRINT, "(GRADING2B)\n");
        if (file->f_mode & FMODE_WRITE){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                if(!(S_ISDIR(file->f_vnode->vn_mode)) && file->f_vnode->vn_ops->write != NULL){
                        int seek_res = 0;

                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        if(file->f_mode & FMODE_APPEND){
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                seek_res = do_lseek(fd,0,SEEK_END);
                        }
                        int bytes = 0;
                        bytes = file->f_vnode->vn_ops->write(file->f_vnode,file->f_pos,buf,nbytes);

                        if(bytes > 0){
                                /* f is a pointer to the corresponding file */
                                /* write is successful and after cursor position has been updated */
                                /* cursor must not go past end of file for these file types */  
                                KASSERT((S_ISCHR(file->f_vnode->vn_mode)) ||
                                (S_ISBLK(file->f_vnode->vn_mode)) ||
                                ((S_ISREG(file->f_vnode->vn_mode)) && (file->f_pos <= file->f_vnode->vn_len)));
                                dbg(DBG_PRINT, "(GRADING2A 3.a)\n");
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                seek_res = do_lseek(fd,bytes,SEEK_CUR); 
                        }
                        fput(file);
                        if (seek_res < 0){
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                return seek_res;
                        }
                        return bytes;
                }
                else{
                        fput(file);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return -EISDIR; /* Is a directory */
                }
        }
        else{
                fput(file);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        /* f is a pointer to the corresponding file */
        /* write is successful and after cursor position has been updated */
        /* cursor must not go past end of file for these file types */                         
        // KASSERT((S_ISCHR(f->f_vnode->vn_mode)) ||
        //         (S_ISBLK(f->f_vnode->vn_mode)) ||
        //         ((S_ISREG(f->f_vnode->vn_mode)) && (f->f_pos <= f->f_vnode->vn_len))) ;
        // NOT_YET_IMPLEMENTED("VFS: do_write");
        // return -1;
}

/*
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't a valid open file descriptor.
 */
int
do_close(int fd)
{
        if (fd < 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; /* Bad file number */
        }
        if (fd >= NFILES){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; /* Bad file number */
        }
        if (curproc->p_files[fd] != NULL){
                fput(curproc->p_files[fd]);
                curproc->p_files[fd] = NULL;
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return 0;
        }
        else{
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; /* Bad file number */
        }

        // NOT_YET_IMPLEMENTED("VFS: do_close");
        // return -1;
}

/* To dup a file:
 *      o fget(fd) to up fd's refcount
 *      o get_empty_fd()
 *      o point the new fd to the same file_t* as the given fd
 *      o return the new file descriptor
 *
 * Don't fput() the fd unless something goes wrong.  Since we are creating
 * another reference to the file_t*, we want to up the refcount.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't an open file descriptor.
 *      o EMFILE
 *        The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int
do_dup(int fd)
{
        if (fd >= 0 && fd < NFILES && curproc->p_files[fd] != NULL){
                file_t *file = NULL;
                file = fget(fd);
                int get_fd = get_empty_fd(curproc);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                if (get_fd < 0){
                        fput(file);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return get_fd;
                }
                else{
                        curproc->p_files[get_fd] = file;
                        // curproc->p_files[get_fd] = curproc->p_files[fd];
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return get_fd;
                }
        }
        else{
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        // NOT_YET_IMPLEMENTED("VFS: do_dup");
        // return -1;
}

/* Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first.  Then return the new file descriptor.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int
do_dup2(int ofd, int nfd)
{
        // if (ofd < 0 || ofd >= NFILES){
        //         dbg(DBG_PRINT, "(GRADING2B)\n");
        //         return -EBADF;
        // }
        // file_t *file = fget(ofd);
        // if (file == NULL){
        //         dbg(DBG_PRINT, "(GRADING2B)\n");
        //         return -EBADF;
        // }

        // if(curproc->p_files[nfd] != NULL){
        //         do_close(nfd);
        //         dbg(DBG_PRINT, "(GRADING2B)\n");
        // }
        // curproc->p_files[nfd] = file;
        // dbg(DBG_PRINT, "(GRADING2B)\n");
        // return nfd;

        if (ofd >= 0 && ofd < NFILES && nfd >= 0 && nfd < NFILES && curproc->p_files[ofd] != NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                if (ofd == nfd){
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return ofd;
                }
                else{
                        int close_res = 0;
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        if(curproc->p_files[nfd] != NULL){
                                close_res = do_close(nfd);
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                        }
                        if (close_res == 0){
                                file_t *file = fget(ofd);
                                curproc->p_files[nfd] = file;
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                return nfd;
                        }
                        else{
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                return close_res;
                        }
                }
        }
        else{
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        // NOT_YET_IMPLEMENTED("VFS: do_dup2");
        // return -1;
}

/*
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'. 'mode' should be one of S_IFCHR or
 * S_IFBLK (you might note that mknod(2) normally allows one to create
 * regular files as well-- for simplicity this is not the case in Weenix).
 * 'devid', as you might expect, is the device identifier of the device
 * that the new special file should represent.
 *
 * You might use a combination of dir_namev, lookup, and the fs-specific
 * mknod (that is, the containing directory's 'mknod' vnode operation).
 * Return the result of the fs-specific mknod, or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        mode requested creation of something other than a device special
 *        file.
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mknod(const char *path, int mode, unsigned devid)
{
        if (S_ISCHR(mode) || S_ISBLK(mode)){
                const char *pathname = NULL;
                size_t len = 0;
                vnode_t *dir_vnode = NULL;
                int dir_res = dir_namev(path,&len,&pathname,NULL,&dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");

                if (dir_res == 0){
                        vnode_t* result = NULL;
                        int lookup_res = lookup(dir_vnode, pathname, len, &result);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        if(lookup_res == 0){
                                vput(result);
                                vput(dir_vnode);
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                return -EEXIST;
                        }
                        else if(lookup_res == -ENOTDIR){
                                vput(dir_vnode);
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                return lookup_res;
                        }
                        else{
                                /* dir_vnode is the directory vnode where you will 
                                create the target special file */
                                KASSERT(NULL != dir_vnode->vn_ops->mknod);
                                dbg(DBG_PRINT, "(GRADING2A 3.b)\n");
                                int res = dir_vnode->vn_ops->mknod(dir_vnode,pathname,len,mode,devid);
                                vput(dir_vnode);
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                return res;
                        }
                }
                else{
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return dir_res;
                }
        }
        else{
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EINVAL; /* Invalid argument */
        }
        // NOT_YET_IMPLEMENTED("VFS: do_mknod");
        // return -1;
}

/* Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. Return what it returns.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mkdir(const char *path)
{
        const char *pathname = NULL;
        size_t len = 0;
        vnode_t *dir_vnode = NULL;
        int dir_res = dir_namev(path, &len, &pathname, NULL, &dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        if(dir_res < 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return dir_res;
        }
        vnode_t* res_vnode = NULL;
        int lookup_res = lookup(dir_vnode,pathname,len,&res_vnode);
        if(lookup_res == 0){
                vput(res_vnode);
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EEXIST;
        }
        else if(lookup_res == -ENOTDIR){
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return lookup_res;
        }
        else{
                /* dir_vnode is the directory vnode where you will create the target directory */
                KASSERT(NULL != dir_vnode->vn_ops->mkdir);
                dbg(DBG_PRINT, "(GRADING2A 3.c)\n");
                int res = dir_vnode->vn_ops->mkdir(dir_vnode,pathname,len);
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return res;
        }
        
        // NOT_YET_IMPLEMENTED("VFS: do_mkdir");
        // return -1;
}

/* Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.  The rmdir v_op will
 * return an error if the dir to be removed does not exist or is not empty, so
 * you don't need to worry about that here. Return the value of the v_op,
 * or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        path has "." as its final component.
 *      o ENOTEMPTY
 *        path has ".." as its final component.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_rmdir(const char *path)
{
        const char* pathname = NULL;
        size_t len = 0;
        vnode_t *dir_vnode = NULL;
        int dir_res = dir_namev(path, &len, &pathname, NULL, &dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        if (dir_res < 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return dir_res;
        }

        if(dir_vnode->vn_ops->rmdir == NULL){
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR;
        }
        if(name_match(pathname, ".", len)){
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EINVAL;
        }
        if(name_match(pathname, "..", len)){
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTEMPTY;
        }

        vnode_t *res_vnode = NULL;
        int lookup_res = lookup(dir_vnode, pathname, len, &res_vnode);
        if(lookup_res != 0){
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return lookup_res;  
        }
        if(res_vnode->vn_ops->rmdir != NULL){
                /* please use TWO consecutive "conforming dbg() calls" for this since 
                this function is not executed if you just start and stop weenix */
                /* dir_vnode is the directory vnode where you will remove the target directory */
                KASSERT(NULL != dir_vnode->vn_ops->rmdir);
                dbg(DBG_PRINT, "(GRADING2A 3.d)\n");
                int res = dir_vnode->vn_ops->rmdir(dir_vnode,pathname,len);
                vput(res_vnode);
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return res;
        }
        else{
                vput(res_vnode);
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR; 
        }
        
        // NOT_YET_IMPLEMENTED("VFS: do_rmdir");
        // return -1;
}

/*
 * Similar to do_rmdir, but for files.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EPERM
 *        path refers to a directory.
 *      o ENOENT
 *        Any component in path does not exist, including the element at the
 *        very end.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_unlink(const char *path)
{
        const char *pathname = NULL;
        size_t len = 0;
        vnode_t *dir_vnode = NULL;
        int dir_res = dir_namev(path, &len, &pathname, NULL, &dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        if (dir_res < 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return dir_res;
        }
        if(dir_vnode->vn_ops->unlink == NULL){
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR;
        }

        vnode_t *res_vnode = NULL;
        int lookup_res = lookup(dir_vnode, pathname, len, &res_vnode);
        if (lookup_res != 0){
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return lookup_res;
        }
        if (res_vnode->vn_ops->rmdir != NULL){
                vput(res_vnode);
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EPERM; /* Operation not permitted */
        }
        else{
                /* please use TWO consecutive "conforming dbg() calls" for this since this function 
                is not executed if you just start and stop weenix */
                /* dir_vnode is the directory vnode where you will unlink the target file */
                KASSERT(NULL != dir_vnode->vn_ops->unlink);
                dbg(DBG_PRINT, "(GRADING2A 3.e)\n");
                int res = dir_vnode->vn_ops->unlink(dir_vnode, pathname, len);
                vput(res_vnode);
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return res;
        }
        // NOT_YET_IMPLEMENTED("VFS: do_unlink");
        // return -1;
}

/* To link:
 *      o open_namev(from)
 *      o dir_namev(to)
 *      o call the destination dir's (to) link vn_ops.
 *      o return the result of link, or an error
 *
 * Remember to vput the vnodes returned from open_namev and dir_namev.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        to already exists.
 *      o ENOENT
 *        A directory component in from or to does not exist.
 *      o ENOTDIR
 *        A component used as a directory in from or to is not, in fact, a
 *        directory.
 *      o ENAMETOOLONG
 *        A component of from or to was too long.
 *      o EPERM
 *        from is a directory.
 */
int
do_link(const char *from, const char *to)
{
        if (strlen(from) > MAXPATHLEN || strlen(to) > MAXPATHLEN){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENAMETOOLONG; /* File name too long */
        }
        vnode_t *from_vnode = NULL;
        int res = 0;
        res = open_namev(from, O_RDONLY, &from_vnode, NULL);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        if (res < 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return res;
        }

        const char *pathname = NULL;
        size_t len = 0;
        vnode_t *to_vnode = NULL;

        res = dir_namev(to, &len, &pathname, NULL, &to_vnode );
        dbg(DBG_PRINT, "(GRADING2B)\n");

        if(res < 0){
                vput(from_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return res;
        }
        if(to_vnode->vn_ops->link == NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR; /* Not a directory */
        }

        vnode_t *res_vnode = NULL;
        res = lookup(to_vnode,pathname,len,&res_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        if(res != 0){
                res = to_vnode->vn_ops->link(from_vnode,to_vnode,pathname,len);
                vput(from_vnode);
                vput(to_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return res;
        }
        else{
                vput(res_vnode);
                vput(from_vnode);
                vput(to_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EEXIST;  /* File exists */
        }


        // NOT_YET_IMPLEMENTED("VFS: do_link");
        // return -1;
}

/*      o link newname to oldname
 *      o unlink oldname
 *      o return the value of unlink, or an error
 *
 * Note that this does not provide the same behavior as the
 * Linux system call (if unlink fails then two links to the
 * file could exist).
 */
int
do_rename(const char *oldname, const char *newname)
{
        int res = 0;
        res = do_link(oldname, newname);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        if (res < 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return res;
        }
        res = do_unlink(oldname);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return res;
        // NOT_YET_IMPLEMENTED("VFS: do_rename");
        // return -1;
}

/* Make the named directory the current process's cwd (current working
 * directory).  Don't forget to down the refcount to the old cwd (vput()) and
 * up the refcount to the new cwd (open_namev() or vget()). Return 0 on
 * success.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        path does not exist.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o ENOTDIR
 *        A component of path is not a directory.
 */
int
do_chdir(const char *path)
{
        vnode_t *dir_vnode = NULL;
        int res = 0;
        res = open_namev(path, O_RDONLY, &dir_vnode, NULL);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        if (res < 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return res;
        }
        
        // ERROR CHECKS
         if (dir_vnode == NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOENT;  /* No such file or directory */
        }
        if( !S_ISDIR(dir_vnode->vn_mode)){
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR;
        }
        if (strlen(path) > MAXPATHLEN){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENAMETOOLONG;
        }

        vput(curproc->p_cwd);
        curproc->p_cwd = dir_vnode;
        // vget(dir_vnode->vn_fs, dir_vnode->vn_vno);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return 0;
        // NOT_YET_IMPLEMENTED("VFS: do_chdir");
        // return -1;
}

/* Call the readdir vn_op on the given fd, filling in the given dirent_t*.
 * If the readdir vn_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.  You need to increment the
 * file_t's f_pos by this amount.  As always, be aware of refcounts, check
 * the return value of the fget and the virtual function, and be sure the
 * virtual function exists (is not null) before calling it.
 *
 * Return either 0 or sizeof(dirent_t), or -errno.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        Invalid file descriptor fd.
 *      o ENOTDIR
 *        File descriptor does not refer to a directory.
 */
int
do_getdent(int fd, struct dirent *dirp)
{
        if (fd < 0 || fd >= NFILES){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; /* Bad file number */
        }
        file_t * file =NULL;
        file = fget(fd);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        // error check
        if (file == NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; /* Bad file number */
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");
        if (file->f_vnode->vn_ops->readdir != NULL){
                int bytes = 0;
                bytes = file->f_vnode->vn_ops->readdir(file->f_vnode, file->f_pos, dirp);
                dbg(DBG_PRINT, "(GRADING2B)\n");

                if (bytes<= 0){
                        fput(file);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return bytes;
                }
                else{
                        int seek = 0;
                        seek = do_lseek(fd, bytes, SEEK_CUR);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        if(seek < 0){
                                fput(file);
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                return seek;
                        }
                        else{
                                fput(file);
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                return sizeof(dirent_t);
                        }
                }
        }
        else{
                fput(file);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR; /* Not a directory */
        }


        // NOT_YET_IMPLEMENTED("VFS: do_getdent");
        // return -1;
}

/*
 * Modify f_pos according to offset and whence.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not an open file descriptor.
 *      o EINVAL
 *        whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int
do_lseek(int fd, int offset, int whence)
{
        if (fd >= NFILES || fd <0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // return -EINVAL; /* Invalid argument */
                return -EBADF;
        }
        file_t *file = NULL;
        file = fget(fd);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        if (file == NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; /* Bad file number */
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");
        if (whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END){
                int pos = file->f_pos;
                dbg(DBG_PRINT, "(GRADING2B)\n");
                
                if (whence == SEEK_SET){
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        pos = offset;
                }
                else if (whence == SEEK_CUR){
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        pos += offset;
                }
                else{
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        pos = file->f_vnode->vn_len + offset; 
                }

                if (pos < 0){
                        fput(file);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return -EINVAL; /* Invalid argument */
                }
                
                dbg(DBG_PRINT, "(GRADING2B)\n");
                file->f_pos = pos;
                fput(file);
                return pos;
        }
        else{
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(file);
                return -EINVAL; /* Invalid argument */
        }
        // NOT_YET_IMPLEMENTED("VFS: do_lseek");
        // return -1;
}

/*
 * Find the vnode associated with the path, and call the stat() vnode operation.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        A component of path does not exist.
 *      o ENOTDIR
 *        A component of the path prefix of path is not a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o EINVAL
 *        path is an empty string.
 */
int
do_stat(const char *path, struct stat *buf)
{
        vnode_t *vn = NULL;
        int res = 0;
        res = open_namev(path, O_RDONLY, &vn, NULL);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        if (res < 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return res;
        }
        else{
                /* please use TWO consecutive "conforming dbg() calls" for this since this 
                function is not executed if you just start and stop weenix */
                /* vn is the vnode where you will perform "stat" */
                KASSERT(NULL != vn->vn_ops->stat);
                dbg(DBG_PRINT, "(GRADING2A 3.f)\n");

                res = vn->vn_ops->stat(vn, buf);
                vput(vn);
                dbg(DBG_PRINT, "(GRADING2A 3.f)\n");
                return res;
        }

        // NOT_YET_IMPLEMENTED("VFS: do_stat");
        // return -1;
}

#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int
do_mount(const char *source, const char *target, const char *type)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
        return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not worry
 * about freeing the fs_t struct here, that is done in vfs_umount. All this function
 * does is figure out which file system to pass to vfs_umount and do good error
 * checking.
 */
int
do_umount(const char *target)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_umount");
        return -EINVAL;
}
#endif
