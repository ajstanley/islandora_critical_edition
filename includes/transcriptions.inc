<?php

/**
 * @file
 * Handles the management of transcription objects.
 */

/**
 * The form for creating a transcription object.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_critical_edition_create_transcription_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'islandora_critical_edition', 'includes/utilities');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  drupal_set_title($object->label);
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('txt');
  $form = array();
  $form['transcription'] = array(
    '#type' => 'textfield',
    '#title' => t('Title of new transcription'),
    '#size' => 40,
    '#default_value' => t('Transcription of') . ' ' . $object->label,
  );

  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $object->id,
  );

  $form['file'] = array(
    '#title' => t('Upload Optional Transcription.'),
    '#type' => 'managed_file',
    '#description' => t('Select file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions,
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create Transcription'),
  );
  return $form;
}

/**
 * Submit handler for Transcription creation.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form_state
 */
function islandora_critical_edition_create_transcription_form_submit(array $form, array &$form_state) {
  $pid = $form_state['values']['pid'];
  $versionable_object = islandora_object_load($pid);
  $label = $form_state['values']['transcription'];
  $tuque = new IslandoraTuque();
  $namespace = islandora_get_namespace($versionable_object->id);
  $transcription = $tuque->repository->constructObject($namespace);
  $transcription->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $versionable_object->id);
  $transcription->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:transcriptionCModel');
  $transcription->label = $label;
  $transcription->owner = $versionable_object->owner;
  $datastream_id = "TRANSCRIPTION";
  $control_group = "M";
  $transcription_datastream = $transcription->constructDatastream($datastream_id, $control_group);
  $transcription_datastream->mimetype = 'text/plain';
  if (!empty($form_state['values']['file'])) {
    $file = file_load($form_state['values']['file']);
    $path = drupal_realpath($file->uri);
    $transcription_datastream->setContentFromFile($path);
    file_delete($file);
  }
  else {
    $transcription_datastream->setContentFromString(t('No transcription entered.'));
  }

  $transcription->ingestDatastream($transcription_datastream);
  $tuque->repository->ingestObject($transcription);
}

/**
 * The form for creating a transcription object.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_critical_edition_manage_transcriptions_form(array $form, array &$form_state, AbstractObject $object) {
  $form = array();
  $rows = array();
  $versionable_object_pid = $object->id;
  $transcriptions = islandora_critical_get_related_transcriptions($versionable_object_pid);
  foreach ($transcriptions as $pid => $label) {
    $rows[$pid] = array(
      'pid' => $pid,
      'title' => $label,
    );
  }
  $header = array(
    'title' => array('data' => t('Transcription Title')),
    'pid' => array('data' => t('PID')),
  );
  // Build and return table element.
  $book_element = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t("This versionable object has not transcription objects."),
    '#js' => FALSE,
    '#multiple' => FALSE,
  );
  $form['intro'] = array(
    '#type' => 'markup',
    '#markup' => t('Choose transcription.'),
  );
  $form['transcription'] = $book_element;
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete Transcription'),
    '#attributes' => array('source' => 'delete_transcription', 'class' => array('transcription_submit')),
  );
  $form['edit'] = array(
    '#type' => 'submit',
    '#value' => t('Edit Transcription'),
    '#attributes' => array('source' => 'edit_transcription', 'class' => array('transcription_submit')),
  );
  return $form;
}

/**
 * Submission handler handler for transcription management form.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function islandora_critical_edition_manage_transcriptions_form_submit(array $form, array &$form_state) {
  module_load_include('module', 'islandora');
  $action = $form_state['triggering_element']['#attributes']['source'];
  if (isset($form_state['values']['transcription'])) {
    $pid = $form_state['values']['transcription'];
  }
  else {
    return;
  }

  switch ($action) {
    case 'delete_transcription':
      islandora_delete_object(islandora_object_load($pid));
      break;

    case 'edit_transcription':
      drupal_goto("islandora/transcription/edit/$pid");
  }
}


/**
 * Form to update transcription.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form_state
 * @param abstractObject $transcription
 *   transcription object
 */
function islandora_critical_transcription_edit_form($form, $form_state, $transcription) {
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($transcription));
  drupal_set_title($transcription->label);
  $form = array();
  $form['transcription'] = array(
    '#title' => t('Current Transcription'),
    '#type' => 'textarea',
    '#default_value' => $transcription['TRANSCRIPTION']->content,
    '#rows' => 40,
    '#cols' => 20,
    '#attributes' => array('class' => array('transcription_window')),
  );
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $transcription->id,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Transcription'),
  );
  return $form;
}

/**
 * Submit handler for transcription edit form.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form
 */
function islandora_critical_transcription_edit_form_submit($form, $form_state) {
  $transcription_object = islandora_object_load($form_state['values']['pid']);
  try {
    $success = $transcription_object['TRANSCRIPTION']->content = $form_state['values']['transcription'];
  }
  catch (Exception $exc) {
    echo $exc->getTraceAsString();
  }
  if ($success) {
    drupal_set_message(t("Transcription updated."));
  }
}
