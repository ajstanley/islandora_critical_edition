<?php

/**
 * @file
 * Handles the association of a CwrcWriter object.
 */

/**
 * The form for associating a CWRCWriter object, tied to a.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_critical_edition_associate_book_form(array $form, array &$form_state) {
  $books = islandora_critical_edition_get_books();
  foreach ($books as $pid => $label) {
    $rows[$pid] = array(
      'pid' => $pid,
      'title' => $label,
    );
  }
  $header = array(
    'title' => array('data' => t('Book Title')),
    'pid' => array('data' => t('PID')),
  );
  // Build and return table element.
  $book_element = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t("There are no book objects in this Fedora Repository."),
    '#js' => FALSE,
    '#multiple' => FALSE,
  );
  $form['intro'] = array(
    '#type' => 'markup',
    '#markup' => t('Choose source book object.'),
  );
  $form['book_objects'] = $book_element;
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Create CWRCWriter'),
  );

  return $form;
  $form = array();
}

/**
 * Submit handler, creates CWRCWriter objects.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_critical_edition_associate_book_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_critical_edition', 'includes/create');
  if (!isset($form_state['values']['book_objects'])) {
    return;
  }
  module_load_include('inc', 'islandora', '/includes/ingest.form');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/utilities');

  $book_pid = $form_state['values']['book_objects'];
  $book_object = islandora_object_load($book_pid);
  $versionable_object = $form_state['islandora']['objects'][0];
  // Create Critical edition Fedora Object.
  $content_models = array(array('pid' => 'islandora:criticalEditionCModel'));

  $fedora_object = islandora_critical_edition_create_object(islandora_get_namespace($book_pid), $book_object->label, $content_models);

  $fedora_object->relationships->add(ISLANDORA_RELS_EXT_URI, ISLANDORA_CRITICAL_EDITION_IS_CRITICAL_EDITION_OF, $book_pid, RELS_TYPE_PLAIN_LITERAL);
  $fedora_object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:criticalEditionCModel');
  $fedora_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $versionable_object->id);
  $edition_object = islandora_add_object($fedora_object);

  $pages = islandora_paged_content_get_pages($book_object);

  $batch = array(
    'title' => t('Creating New CWRCWriter'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'islandora_critical_edition') . '/includes/create.inc',
    'progress_message' => t('@current of @total pages created.'),
  );

  foreach ($pages as $page) {
    $batch['operations'][] = array('islandora_create_edition_page', array(
        'page' => $page,
        'parent_book' => $edition_object->id));
  }

  batch_set($batch);
  batch_process();
}

/**
 * Returns all book objects in the Fedora Repository.
 *
 * @return array
 *   array of PID => Label pairs
 */
function islandora_critical_edition_get_books() {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tuque = islandora_get_tuque_connection();
  $query = "PREFIX fm: <info:fedora/fedora-system:def/model#>
                SELECT DISTINCT ?subject ?label FROM <#ri>
                WHERE {
                        {?subject fm:hasModel <info:fedora/islandora:bookCModel>;
                       }
                 OPTIONAL{
                           ?subject fm:label  ?label; 
                          }
                       }";
  $books = array();
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  foreach ($results as $result) {
    $books[$result['subject']['value']] = $result['label']['value'];
  }
  return $books;
}