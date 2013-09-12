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
  module_load_include('inc', 'islandora_critical_edition', 'includes/transcriptions');
  module_load_include('inc', 'islandora_critical_edition', 'includes/utilities');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  drupal_set_title($object->label);
  $members = islandora_critical_edition_get_members($object->id);
  $critical_edition = array_search('islandora:criticalEditionCModel', $members);
  $transcriptions = array_keys($members, 'islandora:transcriptionCModel');

  $output = array();
  $output['manage_pages'] = array('#type' => 'vertical_tabs');
  $output['manage_pages']['create_cwrcwriter'] = array(
    '#access' => user_access(ISLANDORA_ADD_DS) & !$critical_edition,
    '#title' => t('Create CWRCWriter'),
    '#description' => t('Create CWRCWriter object from existing book object'),
    '#type' => 'fieldset',
    'form' => drupal_get_form('islandora_critical_edition_associate_book_form', $object),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $output['manage_pages']['add_transcription'] = array(
    '#access' => user_access(ISLANDORA_ADD_DS),
    '#title' => t('Create Transcription'),
    '#type' => 'fieldset',
    'form' => drupal_get_form('islandora_critical_edition_create_transcription_form', $object),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $output['manage_pages']['manage_transcription'] = array(
    '#access' => user_access(ISLANDORA_ADD_DS) && $transcriptions,
    '#title' => t('Manage Transcriptions'),
    '#type' => 'fieldset',
    'form' => drupal_get_form('islandora_critical_edition_manage_transcriptions_form', $object),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $output['manage_pages']['delete_cwrcwriter'] = array(
    '#access' => user_access(ISLANDORA_ADD_DS) && $critical_edition,
    '#title' => t('Delete CWRCWriter'),
    '#type' => 'fieldset',
    'form' => drupal_get_form('islandora_critical_edition_delete_cwrcwriter_form', $object, $critical_edition),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  return $output;
}
