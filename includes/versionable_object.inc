<?php

/**
 * @file
 * Files to manage versionable objects.
 */

/**
 * Displays the versionable object forms in a set of vertical tabs.
 *
 * @param AbstractObject $object
 *   The versionable object to manage.
 *
 * @return array
 *   A renderable array containing all the management forms related to
 *   versionable objects.
 */
function islandora_versionable_object_management_menu(AbstractObject $object) {
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  module_load_include('inc', 'islandora_critical_edition', 'includes/create_cwrcwriter.form');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  drupal_set_title($object->label);
  return array(
    'manage_pages' => array(
      '#type' => 'vertical_tabs',
      'create_cwrcwriter' => array(
        '#access' => user_access(ISLANDORA_ADD_DS),
        '#title' => t('Create CWRCWriter from existing book'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_critical_edition_associate_book_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
    ),
  );
}


