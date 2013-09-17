<?php

/**
 * @file
 * Handles the association of a CwrcWriter object.
 */

/**
 * The form for associating a CWRCWriter object.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_critical_edition_associate_book_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'islandora_critical_edition', 'includes/utilities');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  drupal_set_title($object->label);
  $has_critical_edition = FALSE;
  $members = islandora_critical_edition_get_members($object->id);
  if (array_search('islandora:criticalEditionCModel', $members)) {
    $has_critical_edition = TRUE;
    $form['intro'] = array(
      '#type' => 'markup',
      '#markup' => t("You may only have a single CWRCwriter associated with a Versionable Object"),
    );
    return $form;
  }
  $books = islandora_critical_edition_get_books();
  $versionable_object_pid = $object->id;
  $rows = array();
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
  $form['versionable_object_pid'] = array(
    '#type' => 'hidden',
    '#value' => $object->id,
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
  // Create Critical edition Fedora Object.
  $versionable_object_pid = $form_state['values']['versionable_object_pid'];
  $versionable_object = islandora_object_load($versionable_object_pid);
  $tn = $book_object['TN'];
  if ($tn) {
    $versionable_object->ingestDatastream($tn);
  }
  $fedora_object = islandora_critical_edition_create_object(islandora_get_namespace($book_pid), $book_object->label);

  $fedora_object->relationships->add(ISLANDORA_RELS_EXT_URI, ISLANDORA_CRITICAL_EDITION_IS_CRITICAL_EDITION_OF, $book_pid, RELS_TYPE_PLAIN_LITERAL);
  $fedora_object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:criticalEditionCModel');
  $fedora_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $versionable_object_pid);
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

/**
 * The form for associating a CWRCWriter object, tied to a.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_critical_edition_delete_cwrcwriter_form(array $form, array &$form_state, AbstractObject $object, $critical_edition) {
  $form_state['critical_edition_pid'] = $critical_edition;
  return confirm_form($form, $object->label, "islandora/object/$object->id", t("Are you sure you want to delete %title? !break This action cannot be undone.", array('%title' => $object->label, '!break' => "<br />")), t('Delete CWRCWriter'), t('Cancel')
  );
}

/**
 * Submit handler for CWRCWriter deletion form.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function islandora_critical_edition_delete_cwrcwriter_form_submit($form, $form_state) {
  module_load_include('inc', 'islandora_critical_edition', 'includes/utilities');
  $critical_edition_pid = $form_state['critical_edition_pid'];
  $pids_to_delete = islandora_critical_edition_get_members($critical_edition_pid);
  $pids_to_delete[] = $critical_edition_pid;
  foreach ($pids_to_delete as $pid) {
    $temp = islandora_object_load($pid);
    $temp->state = "D";
  }
}
