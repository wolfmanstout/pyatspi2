/*
 * AT-SPI - Assistive Technology Service Provider Interface
 * (Gnome Accessibility Project; http://developer.gnome.org/projects/gap)
 *
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * accessible.c: test for accessibility implementation
 *
 */
#include <config.h>
#include <bonobo/Bonobo.h>

#include <stdio.h>

/*
 * This pulls the CORBA definitions for the "Accessibility::Accessible" server
 */
#include <libspi/Accessibility.h>

/*
 * This pulls the definition for the BonoboObject (Gtk Type)
 */
#include "accessible.h"
#include "component.h"
#include "editabletext.h"
#include "hyperlink.h"
#include "hypertext.h"
#include "image.h"
#include "selection.h"
#include "table.h"
#include "text.h"
#include "value.h"

/*
 * Our parent Gtk object type
 */
#define PARENT_TYPE BONOBO_OBJECT_TYPE

/*
 * A pointer to our parent object class
 */
static GObjectClass *accessible_parent_class;

/*
 * Implemented GObject::finalize
 */
static void
accessible_object_finalize (GObject *object)
{
        Accessible *accessible = ACCESSIBLE (object);

        printf("accessible_object_finalize called\n");
	ATK_OBJECT (accessible->atko); /* assertion */
        g_object_unref (G_OBJECT(accessible->atko));
        accessible->atko = NULL;

        printf("atko freed, calling parent finalize\n");
        accessible_parent_class->finalize (object);
}

/*
 * CORBA Accessibility::Accessible::get_name method implementation
 */
static CORBA_char *
impl_accessibility_accessible_get_name (PortableServer_Servant servant,
                                        CORBA_Environment     *ev)
{
  CORBA_char * retval;
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  retval = (CORBA_char *) atk_object_get_name (accessible->atko);
  if (retval )
    retval = CORBA_string_dup (retval);
  else
    retval = CORBA_string_dup ("");

  return retval;
}

/*
 * CORBA Accessibility::Accessible::set_name method implementation
 */
static void
impl_accessibility_accessible_set_name (PortableServer_Servant servant,
                                        const CORBA_char      *name,
                                        CORBA_Environment     *ev)
{
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  atk_object_set_name (accessible->atko, name);
  printf ("Accessible set_name called: %s\n", name);
}

/*
 * CORBA Accessibility::Accessible::get_description method implementation
 */
static CORBA_char *
impl_accessibility_accessible_get_description (PortableServer_Servant servant,
                                               CORBA_Environment     *ev)
{
  CORBA_char * retval;
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  retval = CORBA_string_dup (atk_object_get_description (accessible->atko));

  return retval;
}

/*
 * CORBA Accessibility::Accessible::set_description method implementation
 */
static void
impl_accessibility_accessible_set_description (PortableServer_Servant servant,
                                               const CORBA_char      *name,
                                               CORBA_Environment     *ev)
{
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  atk_object_set_description (accessible->atko, name);
  printf ("Accessible set_description called: %s\n", name);
}

/*
 * CORBA Accessibility::Accessible::get_parent method implementation
 */
static Accessibility_Accessible
impl_accessibility_accessible_get_parent (PortableServer_Servant servant,
                                          CORBA_Environment     *ev)
{
  Accessibility_Accessible retval;
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  AtkObject *parent;
  parent = atk_object_get_parent (accessible->atko);
  retval = BONOBO_OBJREF (accessible_new (parent));
  printf ("Accessible get_parent called\n");
  return CORBA_Object_duplicate (retval, &ev);
}

/*
 * CORBA Accessibility::Accessible::get_IndexInParent method implementation
 */
static CORBA_long
impl_accessibility_accessible_get_index_in_parent (PortableServer_Servant servant,
                                                   CORBA_Environment     *ev)
{
  CORBA_long retval;
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  retval = (CORBA_long) atk_object_get_index_in_parent (accessible->atko);
  printf ("Accessible get_index_in_parent called\n");
  return retval;
}

/*
 * CORBA Accessibility::Accessible::get_childCount method implementation
 */
static CORBA_long
impl_accessibility_accessible_get_child_count (PortableServer_Servant servant,
                                               CORBA_Environment     *ev)
{
  CORBA_long retval;
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  retval = (CORBA_long) atk_object_get_n_accessible_children (accessible->atko);
  printf ("Accessible get_childCount called: %d\n", (int) retval);
  return retval;
}

/*
 * CORBA Accessibility::Accessible::getChildAtIndex method implementation
 */
static Accessibility_Accessible
impl_accessibility_accessible_get_child_at_index (PortableServer_Servant servant,
                                                  const CORBA_long      index,
                                                  CORBA_Environment     *ev)
{
  Accessibility_Accessible retval;
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  AtkObject *child = atk_object_ref_accessible_child (accessible->atko, (gint) index);
  retval = BONOBO_OBJREF (accessible_new (child));
  printf ("Accessible get_child_at_index called.\n");
  return CORBA_Object_duplicate (retval, &ev);
}

/*
 * CORBA Accessibility::Accessible::getState method implementation
 */
static Accessibility_StateSet
impl_accessibility_accessible_get_state (PortableServer_Servant servant,
					 CORBA_Environment     *ev)
{
  Accessibility_StateSet retval;
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  AtkStateSet *state = atk_object_ref_state_set (accessible->atko);
  retval = CORBA_OBJECT_NIL;
  printf ("Accessible get_state.\n");
  /* TODO: implement the bonobo stateset class */
  return (Accessibility_StateSet) retval;
}

/*
 * CORBA Accessibility::Accessible::getRelationSet method implementation
 */
static Accessibility_RelationSet *
impl_accessibility_accessible_get_relation_set (PortableServer_Servant servant,
						CORBA_Environment     *ev)
{
  Accessibility_RelationSet *retval;
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  AtkRelationSet *relation_set = atk_object_ref_relation_set (accessible->atko);
  retval = CORBA_sequence_Accessibility_Relation__alloc ();
  /*
   *  TODO: fill the sequence with relation set objects, themselves
   *  initialized from the AtkRelation object in the AtkRelationSet.
   */
  printf ("Accessible get_relation_set.\n");
  return retval;
}

/*
 * CORBA Accessibility::Accessible::getRole method implementation
 */
static Accessibility_Role
impl_accessibility_accessible_get_role (PortableServer_Servant servant,
					CORBA_Environment     *ev)
{
  Accessibility_Role retval;
  Accessible *accessible = ACCESSIBLE (bonobo_object_from_servant (servant));
  AtkRole role = atk_object_get_role (accessible->atko);
  retval = role;
  printf ("Accessible get_role.\n");
  return (Accessibility_Role) retval;
}

static void
accessible_class_init (AccessibleClass *klass)
{
        GObjectClass * object_class = (GObjectClass *) klass;
        POA_Accessibility_Accessible__epv *epv = &klass->epv;
        accessible_parent_class = g_type_class_peek_parent (klass);

        object_class->finalize = accessible_object_finalize;

        epv->_get_name = impl_accessibility_accessible_get_name;
        epv->_set_name = impl_accessibility_accessible_set_name;
        epv->_get_description = impl_accessibility_accessible_get_description;
        epv->_set_description = impl_accessibility_accessible_set_description;

        epv->_get_parent = impl_accessibility_accessible_get_parent;
        epv->_get_childCount = impl_accessibility_accessible_get_child_count;
        epv->getChildAtIndex = impl_accessibility_accessible_get_child_at_index;
        epv->getIndexInParent = impl_accessibility_accessible_get_index_in_parent;

        epv->getRelationSet = impl_accessibility_accessible_get_relation_set;
        epv->getState = impl_accessibility_accessible_get_state;
        epv->getRole = impl_accessibility_accessible_get_role;
}

static void
accessible_init (Accessible *accessible)
{
}

GType
accessible_get_type (void)
{
        static GType type = 0;

        if (!type) {
                static const GTypeInfo tinfo = {
                        sizeof (AccessibleClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) accessible_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL, /* class data */
                        sizeof (Accessible),
                        0, /* n preallocs */
                        (GInstanceInitFunc) accessible_init,
                        NULL /* value table */
                };
                /*
                 * Bonobo_type_unique auto-generates a load of
                 * CORBA structures for us. All derived types must
                 * use bonobo_type_unique.
                 */
                type = bonobo_type_unique (
                        PARENT_TYPE,
                        POA_Accessibility_Accessible__init,
                        NULL,
                        G_STRUCT_OFFSET (AccessibleClass, epv),
                        &tinfo,
                        "Accessible");
        }

        return type;
}

Accessible *
accessible_new (AtkObject *o)
{
    Accessible *retval =
               ACCESSIBLE (g_object_new (accessible_get_type (), NULL));
    CORBA_Environment ev;
    CORBA_exception_init (&ev);
    g_object_ref (o);
    retval->atko = ATK_OBJECT (o);

    /*
     * TODO: add interface containers/constructors for EDITABLE_TEXT, HYPERTEXT,
     *  IMAGE, SELECTION, TABLE, TEXT, VALUE.
     */

    /* add appropriate ATK interfaces */

    if (ATK_IS_ACTION (o))
      {
        bonobo_object_add_interface (bonobo_object (retval),
                                     BONOBO_OBJECT (action_interface_new (o)));
      }

    if (ATK_IS_COMPONENT (o))
      {
        bonobo_object_add_interface (bonobo_object (retval),
                                     BONOBO_OBJECT (component_interface_new (o)));
      }

    if (ATK_IS_EDITABLE_TEXT (o))
      {
        bonobo_object_add_interface (bonobo_object (retval),
                                     BONOBO_OBJECT(editable_text_interface_new (o)));
      }

    else if (ATK_IS_HYPERTEXT (o))
      {
        bonobo_object_add_interface (bonobo_object (retval),
                                     BONOBO_OBJECT (hypertext_interface_new (o)));
      }

    else if (ATK_IS_TEXT (o))
      {
        bonobo_object_add_interface (bonobo_object (retval),
                                     BONOBO_OBJECT (text_interface_new (o)));
      }

    if (ATK_IS_IMAGE (o))
      {
        bonobo_object_add_interface (bonobo_object (retval),
                                     BONOBO_OBJECT (image_interface_new (o)));
      }

    if (ATK_IS_SELECTION (o))
      {
        bonobo_object_add_interface (bonobo_object (retval),
                                     BONOBO_OBJECT (selection_interface_new (o)));
      }

    if (ATK_IS_TABLE (o))
      {
        bonobo_object_add_interface (bonobo_object (retval),
                                     BONOBO_OBJECT (table_interface_new (o)));
      }

    if (ATK_IS_VALUE (o))
      {
        bonobo_object_add_interface (bonobo_object (retval),
                                     BONOBO_OBJECT (value_interface_new (o)));
      }

    return retval;
}
