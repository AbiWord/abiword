/* LibAbi
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef __ABI_DOC_H__
#define __ABI_DOC_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define ABI_TYPE_DOC                  (abi_doc_get_type ())
#define ABI_DOC(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), ABI_TYPE_DOC, AbiDoc))
#define ABI_DOC_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), ABI_TYPE_DOC, AbiDocClass))
#define ABI_IS_DOC(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ABI_TYPE_DOC))
#define ABI_IS_DOC_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), ABI_TYPE_DOC))
#define ABI_DOC_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), ABI_TYPE_DOC, AbiDocClass))


typedef struct _AbiDoc       AbiDoc;
typedef struct _AbiDocClass  AbiDocClass;


GType   abi_doc_get_type         (void) G_GNUC_CONST;
AbiDoc* abi_doc_new              (void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __ABI_DOC_H__ */
