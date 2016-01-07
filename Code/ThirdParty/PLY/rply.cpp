#include "rply.h"
#include "rply_templcode.h"

/* ----------------------------------------------------------------------
* Read support functions
* ---------------------------------------------------------------------- */
p_ply ply_open(const char *name, p_ply_error_cb error_cb, 
  long idata, void *pdata) {
  FILE *fp = NULL; 
  p_ply ply = ply_alloc();
  if (error_cb == NULL) error_cb = ply_error_cb;
  if (!ply) {
    error_cb(NULL, "Out of memory");
    return NULL;
  }
  ply->idata = idata;
  ply->pdata = pdata;
  ply->io_mode = PLY_READ;
  ply->error_cb = error_cb;
  if (!ply_type_check()) {
    error_cb(ply, "Incompatible type system");
    free(ply);
    return NULL;
  }
  assert(name);
  fopen_s(&fp, name, "rb");
  if (!fp) {
    error_cb(ply, "Unable to open file");
    free(ply);
    return NULL;
  }
  ply->fp = fp;
  return ply;
}

int ply_read_header(p_ply ply) {
  assert(ply && ply->fp && ply->io_mode == PLY_READ);
  if (!ply_read_header_magic(ply)) return 0;
  if (!ply_read_word(ply)) return 0;
  /* parse file format */
  if (!ply_read_header_format(ply)) {
    ply_ferror(ply, "Invalid file format");
    return 0;
  }
  /* parse elements, comments or obj_infos until the end of header */
  while (strcmp(BWORD(ply), "end_header")) {
    if (!ply_read_header_comment(ply) && 
      !ply_read_header_element(ply) && 
      !ply_read_header_obj_info(ply)) {
        ply_ferror(ply, "Unexpected token '%s'", BWORD(ply));
        return 0;
    }
  }
  /* skip extra character? */
  if (ply->rn) {
    if (BSIZE(ply) < 1 && !BREFILL(ply)) {
      ply_ferror(ply, "Unexpected end of file");
      return 0;
    }
    BSKIP(ply, 1);
  }
  return 1;
}

long ply_set_read_cb(p_ply ply, const char *element_name, 
  const char* property_name, p_ply_read_cb read_cb, 
  void *pdata, long idata) {
  p_ply_element element = NULL; 
  p_ply_property property = NULL;
  assert(ply && element_name && property_name);
  element = ply_find_element(ply, element_name);
  if (!element) return 0;
  property = ply_find_property(element, property_name);
  if (!property) return 0;
  property->read_cb = read_cb;
  property->pdata = pdata;
  property->idata = idata;
  return (int) element->ninstances;
}

int ply_read(p_ply ply) {
  long i;
  p_ply_argument argument;
  assert(ply && ply->fp && ply->io_mode == PLY_READ);
  argument = &ply->argument;
  /* for each element type */
  for (i = 0; i < ply->nelements; i++) {
    p_ply_element element = &ply->element[i];
    argument->element = element;
    if (!ply_read_element(ply, element, argument))
      return 0;
  }
  return 1;
}



int ply_add_element(p_ply ply, const char *name, long ninstances) {
  p_ply_element element = NULL;
  assert(ply && ply->fp && ply->io_mode == PLY_WRITE);
  assert(name && strlen(name) < WORDSIZE && ninstances >= 0);
  if (strlen(name) >= WORDSIZE || ninstances < 0) {
    ply_ferror(ply, "Invalid arguments");
    return 0;
  }
  element = ply_grow_element(ply);
  if (!element) return 0;
  strcpy(element->name, name);
  element->ninstances = ninstances;
  return 1;
}

int ply_add_scalar_property(p_ply ply, const char *name, e_ply_type type) {
  p_ply_element element = NULL;
  p_ply_property property = NULL;
  assert(ply && ply->fp && ply->io_mode == PLY_WRITE);
  assert(name && strlen(name) < WORDSIZE);
  assert(type < PLY_LIST);
  if (strlen(name) >= WORDSIZE || type >= PLY_LIST) {
    ply_ferror(ply, "Invalid arguments");
    return 0;
  }
  element = &ply->element[ply->nelements-1];
  property = ply_grow_property(ply, element);
  if (!property) return 0;
  strcpy(property->name, name);
  property->type = type;
  return 1;
}

int ply_add_list_property(p_ply ply, const char *name, 
  e_ply_type length_type, e_ply_type value_type) {
  p_ply_element element = NULL;
  p_ply_property property = NULL;
  assert(ply && ply->fp && ply->io_mode == PLY_WRITE);
  assert(name && strlen(name) < WORDSIZE);
  if (strlen(name) >= WORDSIZE) {
    ply_ferror(ply, "Invalid arguments");
    return 0;
  }
  assert(length_type < PLY_LIST);
  assert(value_type < PLY_LIST);
  if (length_type >= PLY_LIST || value_type >= PLY_LIST) {
    ply_ferror(ply, "Invalid arguments");
    return 0;
  }
  element = &ply->element[ply->nelements-1];
  property = ply_grow_property(ply, element);
  if (!property) return 0;
  strcpy(property->name, name);
  property->type = PLY_LIST;
  property->length_type = length_type;
  property->value_type = value_type;
  return 1;
}

int ply_add_property(p_ply ply, const char *name, e_ply_type type,
  e_ply_type length_type, e_ply_type value_type) {
    if (type == PLY_LIST) 
      return ply_add_list_property(ply, name, length_type, value_type);
    else 
      return ply_add_scalar_property(ply, name, type);
}

int ply_add_comment(p_ply ply, const char *comment) {
  char *new_comment = NULL;
  assert(ply && comment && strlen(comment) < LINESIZE);
  if (!comment || strlen(comment) >= LINESIZE) {
    ply_ferror(ply, "Invalid arguments");
    return 0;
  }
  new_comment = (char *) ply_grow_array(ply, (void **) &ply->comment,
    &ply->ncomments, LINESIZE);
  if (!new_comment) return 0;
  strcpy(new_comment, comment);
  return 1;
}

int ply_add_obj_info(p_ply ply, const char *obj_info) {
  char *new_obj_info = NULL;
  assert(ply && obj_info && strlen(obj_info) < LINESIZE);
  if (!obj_info || strlen(obj_info) >= LINESIZE) {
    ply_ferror(ply, "Invalid arguments");
    return 0;
  }
  new_obj_info = (char *) ply_grow_array(ply, (void **) &ply->obj_info,
    &ply->nobj_infos, LINESIZE);
  if (!new_obj_info) return 0;
  strcpy(new_obj_info, obj_info);
  return 1;
}

int ply_write_header(p_ply ply) {
  long i, j;
  assert(ply && ply->fp && ply->io_mode == PLY_WRITE);
  assert(ply->element || ply->nelements == 0); 
  assert(!ply->element || ply->nelements > 0); 
  if (fprintf(ply->fp, "ply\nformat %s 1.0\n", 
    ply_storage_mode_list[ply->storage_mode]) <= 0) goto error;
  for (i = 0; i < ply->ncomments; i++)
    if (fprintf(ply->fp, "comment %s\n", ply->comment + LINESIZE*i) <= 0)
      goto error;
  for (i = 0; i < ply->nobj_infos; i++)
    if (fprintf(ply->fp, "obj_info %s\n", ply->obj_info + LINESIZE*i) <= 0)
      goto error;
  for (i = 0; i < ply->nelements; i++) {
    p_ply_element element = &ply->element[i];
    assert(element->property || element->nproperties == 0); 
    assert(!element->property || element->nproperties > 0); 
    if (fprintf(ply->fp, "element %s %ld\n", element->name, 
      element->ninstances) <= 0) goto error;
    for (j = 0; j < element->nproperties; j++) {
      p_ply_property property = &element->property[j];
      if (property->type == PLY_LIST) {
        if (fprintf(ply->fp, "property list %s %s %s\n", 
          ply_type_list[property->length_type],
          ply_type_list[property->value_type],
          property->name) <= 0) goto error;
      } else {
        if (fprintf(ply->fp, "property %s %s\n", 
          ply_type_list[property->type],
          property->name) <= 0) goto error;
      }
    }
  }
  return fprintf(ply->fp, "end_header\n") > 0;
error:
  ply_ferror(ply, "Error writing to file");
  return 0;
}

int ply_write(p_ply ply, double value) {
  p_ply_element element = NULL;
  p_ply_property property = NULL;
  int type = -1;
  int breakafter = 0;
  if (ply->welement > ply->nelements) return 0;
  element = &ply->element[ply->welement];
  if (ply->wproperty > element->nproperties) return 0;
  property = &element->property[ply->wproperty];
  if (property->type == PLY_LIST) {
    if (ply->wvalue_index == 0) {
      type = property->length_type;
      ply->wlength = (long) value;
    } else type = property->value_type;
  } else {
    type = property->type;
    ply->wlength = 0;
  }
  if (!ply->odriver->ohandler[type](ply, value)) {
    ply_ferror(ply, "Failed writing %s of %s %d (%s: %s)", 
      property->name, element->name, 
      ply->winstance_index, 
      ply->odriver->name, ply_type_list[type]);
    return 0;
  }
  ply->wvalue_index++;
  if (ply->wvalue_index > ply->wlength) {
    ply->wvalue_index = 0;
    ply->wproperty++;
  }
  if (ply->wproperty >= element->nproperties) {
    ply->wproperty = 0;
    ply->winstance_index++;
    if (ply->storage_mode == PLY_ASCII) breakafter = 1;
  }
  if (ply->winstance_index >= element->ninstances) {
    ply->winstance_index = 0;
    ply->welement++;
  }
  return !breakafter || putc('\n', ply->fp) > 0;
}

int ply_close(p_ply ply) {
  long i;
  assert(ply && ply->fp);
  assert(ply->element || ply->nelements == 0);
  assert(!ply->element || ply->nelements > 0);
  /* write last chunk to file */
  if (ply->io_mode == PLY_WRITE && 
    fwrite(ply->buffer, 1, ply->buffer_last, ply->fp) < ply->buffer_last) {
      ply_ferror(ply, "Error closing up");
      return 0;
  }
  fclose(ply->fp);
  /* free all memory used by handle */
  if (ply->element) {
    for (i = 0; i < ply->nelements; i++) {
      p_ply_element element = &ply->element[i];
      if (element->property) free(element->property);
    }
    free(ply->element);
  }
  if (ply->obj_info) free(ply->obj_info);
  if (ply->comment) free(ply->comment);
  free(ply);
  return 1;
}


/* ----------------------------------------------------------------------
* Query support functions
* ---------------------------------------------------------------------- */
p_ply_element ply_get_next_element(p_ply ply, 
  p_ply_element last) {
  assert(ply);
  if (!last) return ply->element;
  last++;
  if (last < ply->element + ply->nelements) return last;
  else return NULL;
}

int ply_get_element_info(p_ply_element element, const char** name,
  long *ninstances) {
  assert(element);
  if (name) *name = element->name;
  if (ninstances) *ninstances = (long) element->ninstances;
  return 1;
}

p_ply_property ply_get_next_property(p_ply_element element, 
  p_ply_property last) {
  assert(element);
  if (!last) return element->property;
  last++;
  if (last < element->property + element->nproperties) return last;
  else return NULL;
}

int ply_get_property_info(p_ply_property property, const char** name,
  e_ply_type *type, e_ply_type *length_type, e_ply_type *value_type) {
  assert(property);
  if (name) *name = property->name;
  if (type) *type = property->type;
  if (length_type) *length_type = property->length_type;
  if (value_type) *value_type = property->value_type;
  return 1;
}

const char *ply_get_next_comment(p_ply ply, const char *last) {
  assert(ply);
  if (!last) return ply->comment; 
  last += LINESIZE;
  if (last < ply->comment + LINESIZE*ply->ncomments) return last;
  else return NULL;
}

const char *ply_get_next_obj_info(p_ply ply, const char *last) {
  assert(ply);
  if (!last) return ply->obj_info; 
  last += LINESIZE;
  if (last < ply->obj_info + LINESIZE*ply->nobj_infos) return last;
  else return NULL;
}


/* ----------------------------------------------------------------------
* Callback argument support functions 
* ---------------------------------------------------------------------- */
int ply_get_argument_element(p_ply_argument argument, 
  p_ply_element *element, long *instance_index) {
  assert(argument);
  if (!argument) return 0;
  if (element) *element = argument->element;
  if (instance_index) *instance_index = argument->instance_index;
  return 1;
}

int ply_get_argument_property(p_ply_argument argument, 
  p_ply_property *property, long *length, long *value_index) {
  assert(argument);
  if (!argument) return 0;
  if (property) *property = argument->property;
  if (length) *length = argument->length;
  if (value_index) *value_index = argument->value_index;
  return 1;
}

int ply_get_argument_user_data(p_ply_argument argument, void **pdata, 
  long *idata) {
  assert(argument);
  if (!argument) return 0;
  if (pdata) *pdata = argument->pdata;
  if (idata) *idata = argument->idata;
  return 1;
}

double ply_get_argument_value(p_ply_argument argument) {
  assert(argument);
  if (!argument) return 0.0;
  return argument->value;
}

int ply_get_ply_user_data(p_ply ply, void **pdata, long *idata) {
  assert(ply);
  if (!ply) return 0;
  if (pdata) *pdata = ply->pdata;
  if (idata) *idata = ply->idata;
  return 1;
}

/* ----------------------------------------------------------------------
* Write support functions
* ---------------------------------------------------------------------- */
p_ply ply_create(const char *name, e_ply_storage_mode storage_mode, 
  p_ply_error_cb error_cb, long idata, void *pdata) {
  FILE *fp = NULL;
  p_ply ply = ply_alloc();
  if (error_cb == NULL) error_cb = ply_error_cb;
  if (!ply) {
    error_cb(NULL, "Out of memory");
    return NULL;
  }
  if (!ply_type_check()) {
    error_cb(ply, "Incompatible type system");
    free(ply);
    return NULL;
  }
  assert(name && storage_mode <= PLY_DEFAULT);
  fopen_s(&fp, name, "wb");
  if (!fp) {
    error_cb(ply, "Unable to create file");
    free(ply);
    return NULL;
  }
  ply->idata = idata;
  ply->pdata = pdata;
  ply->io_mode = PLY_WRITE;
  if (storage_mode == PLY_DEFAULT) storage_mode = ply_arch_endian();
  if (storage_mode == PLY_ASCII) ply->odriver = &ply_odriver_ascii;
  else if (storage_mode == ply_arch_endian()) 
    ply->odriver = &ply_odriver_binary;
  else ply->odriver = &ply_odriver_binary_reverse;
  ply->storage_mode = storage_mode;
  ply->fp = fp;
  ply->error_cb = error_cb;
  return ply;
}