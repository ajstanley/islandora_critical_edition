<?php

/**
 * @file
 *   Form to create new versionable object.
 */

/**
 * Creates Versionable Object
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 *
 * @return array
 *   From to be processed
 */
function islandora_critical_edition_create_versionable_object($form, $form_state, $object) {
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  drupal_set_title($object->label);
  $form = array();
  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label for Versionable Object')
  );
  $form['critical_edition_pid'] = array(
    '#type' => 'hidden',
    '#value' => $object->id,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create Versionable Object'),
  );
  return $form;
}

function islandora_critical_edition_create_versionable_object_submit($form, $form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $critical_edition_pid = $form_state['values']['critical_edition_pid'];
  $fedora_object = islandora_object_load($critical_edition_pid);
  $tuque = new IslandoraTuque();
  $namespace = islandora_get_namespace($fedora_object->id);
  $versionable_object = $tuque->repository->constructObject($namespace);
  $versionable_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $fedora_object->id);
  $versionable_object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:versionableObjectCModel');
  $versionable_object->label = $form_state['values']['label'];
  $versionable_object->owner = $fedora_object->owner;
  $tn_path = drupal_get_path('module', 'islandora_critical_edition') . '/images/folder.png';
  $tn = $versionable_object->constructDatastream('TN');
  $tn->label = 'Thumbnail';
  $tn->mimetype = 'image/png';
  $tn->setContentFromFile($tn_path);
  $versionable_object->ingestDatastream($tn);
  $mods = $versionable_object->constructDatastream('MODS');
  $mods->label = 'MODS';
  $mods->mimetype = 'text/xml';
  $mods->setContentFromString("<mods></mods>");
  $versionable_object->ingestDatastream($mods);
  $tuque->repository->ingestObject($versionable_object);
  drupal_goto("islandora/object/$critical_edition_pid");
}