<?php

/**
 * @file
 * Forms for populating Critical Apparatus
 */

/**
 * Form to populate Critical Apparatus.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 * @param AbstractObject $critical_edition
 *   object representing Critical Edition
 *
 * @return array
 *   Drupal form
 */
function critical_apparatus_form($form, $form_state, $critical_edition) {
  module_load_include('inc', 'islandora_critical_edition', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  $apparatus_pid = key(islandora_critical_edition_get_members($critical_edition->id, 'islandora:criticalApparatusCModel'));
  $apparatus = islandora_object_load($apparatus_pid);
  drupal_set_breadcrumb(islandora_get_breadcrumbs($critical_edition));
  drupal_set_title($critical_edition->label);
  $datastreams = islandora_critical_apparatus_get_datastreams();
  $content = "Choose datastream";
  $chosen = isset($form_state['values']['datastream']) ? $form_state['values']['datastream'] : 'title_page';
  $content = t('Enter text for @datastream', array('@datastream' => $datastreams[$chosen]));
  if ($apparatus[$chosen]) {
    $content = $apparatus[$chosen]->content;
  }


  $form = array();
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $apparatus_pid,
  );

  $form['datastream'] = array(
    '#title' => t('Select Component'),
    '#type' => 'select',
    '#options' => $datastreams,
    '#default_value' => $chosen,
    '#ajax' => array(
      'callback' => 'datastream_ajax_callback',
      'wrapper' => 'wrapper',
      'effect' => 'fade',
      'method' => 'replace',
    ),
  );

  $form['wrapper'] = array(
    '#prefix' => '<div id="wrapper">',
    '#suffix' => '</div>',
    '#type' => 'fieldset',
  );

  $form['wrapper'][$chosen] = array(
    '#title' => t('Enter text'),
    '#type' => 'textarea',
    '#default_value' => $content,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create / Update'),
  );

  return $form;
}

/**
 * Ajax callback.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 *
 * @return array
 *   element to be re-rendered
 */
function datastream_ajax_callback($form, $form_state) {
  return $form['wrapper'];
}

/**
 * Returns array of possible datastreams.
 *
 * @return array
 *   Associative array of dsid => label
 */
function islandora_critical_apparatus_get_datastreams() {
  return array(
    'title_page' => t('Title Page'),
    'epigraph' => t('Epigraph'),
    'preface' => t('Preface'),
    'forewword' => t('Foreword'),
    'introduction' => t('Introduction'),
    'illustrations' => t('List of Illustrations, Figures, Graphs'),
    'acknowledgements' => t('Acknowledgements'),
    'editorial_introduction' => t('Editorial Introduction'),
    'headnote' => t('Headnote'),
    'footnote' => t('Footnotes'),
    'endnotes' => t('Endnotes'),
    'epilogue' => t('Epilogue'),
    'afterword' => t('Afterword'),
    'editorial_procedure' => t('Editorial Procedure'),
  );
}

/**
 * Submit handler for apparatus form.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function critical_apparatus_form_submit($form, $form_state) {
  $apparatus = islandora_object_load($form_state['values']['pid']);
  $chosen = $form_state['values']['datastream'];
  $content = $form_state['values'][$chosen];
  if ($apparatus[$chosen]) {
    $apparatus[$chosen]->content = $content;
  }
  else {
    $datastream = $apparatus->constructDatastream($chosen, 'M');
    $datastream->setContentFromString($content);
    $datastream->label = $chosen;
    $datastream->mimetype = 'text/plain';
    $apparatus->ingestDatastream($datastream);
  }
}
