<?php
/**
 * @file
 *   Holds the ajax callbacks for the cwrcwriter.
 */

/**
 * Provides all the parameters necessary for CWRCWriter to operate with Fedora.
 *
 * @XXX There is no 7.x authority module yet. Alan knows how to integrate.
 *   @code
 *     $results['authority_url'] = url();
 *     $results['authority_mappings'] =
 *       islandora_critical_edition_get_authority_mappings();
 *   @endcode
 *
 * @global User $user
 *
 * @param AbstractObject $fedora_object
 *   The object to get the info on for the CWRC writer.
 */
function islandora_critical_edition_setup(AbstractObject $fedora_object) {
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  global $user;

  $fedora_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');

  $critical_edition_relationships = $fedora_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
  $critical_edition = $critical_edition_relationships[0]['object']['value'];
  $critical_edition_object = islandora_object_load($critical_edition);
  $title = $critical_edition_object->label;
  $pages = islandora_paged_content_get_pages($critical_edition_object);
  $pages = array_keys($pages);
  $position = array_search($fedora_object->id, $pages);
  $results = array();
  $results['uid'] = $user->uid;
  $results['fedora_url'] = $fedora_url;
  $results['position'] = $position;
  $results['pages'] = $pages;
  $results['title'] = $title;
  $results['no_edit'] = !islandora_critical_edition_edit_cwrc_access($fedora_object);
  $results['page_count'] = count($pages);
  $json = json_encode($results);
  echo $json;
}

/**
 * Persists CWRC datastream to a Critical Edition object.
 *
 * @post text
 *   The contents of the CWRC datastream.
 *
 * @param AbstractObject $fedora_object
 *   The fedora object to upload the CWRC datastream to.
 */
function islandora_critical_edition_savedata(AbstractObject $fedora_object) {

  $data = html_entity_decode(stripslashes($_POST['text']), ENT_QUOTES, 'UTF-8');
  $cwrc = str_replace('<br>', '<br />', $data);
  $cwrc = str_replace('&', '&amp;', $cwrc);

  $fedora_object["CWRC"]->content = $cwrc;

  echo "Success";
}

/**
 * Ajax callback for Shared Canvas
 * Supplies the basic data, including manifest URI.
 *
 * @param AbstractObject $fedora_object
 *   The Fedora object to supply the data for
 */
function islandora_critical_edition_canvas_setup(AbstractObject $fedora_object) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/build_streams');
  module_load_include('inc', 'islandora_image_annotation', 'includes/rdf_builder');
  module_load_include('inc', 'islandora_image_annotation', 'includes/callbacks');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');


  $relationships = $fedora_object->relationships->get();
  foreach ($relationships as $relationship) {
    if ($relationship['predicate']['value'] == 'hasModel') {
      $content_model = $relationship['object']['value'];
    }
    if ($relationship['predicate']['value'] == 'isEditionOf') {
      $page_object = islandora_object_load($relationship['object']['value']);
    }
  }

  // Establish categories for dropdown.
  $dropdown = variable_get('islandora_annotation_enforce_taxonomy', TRUE);
  $mappings = variable_get('islandora_annotation_mappings', array());
  $vid = isset($mappings[$content_model]['TAX'])? $mappings[$content_model]['TAX']: NULL;
  $categories = array();
  if ($dropdown && isset($vid) && module_exists('taxonomy')) {
    $terms = taxonomy_get_tree($vid, 0, 1);
    foreach ($terms as $term) {
      $categories[] = $term->name;
    }
  }
  else {
    $categories = islandora_annotations_get_type_terms();
  }
  if (empty($categories)) {
    $categories[] = 'unclassified';
  }

  $page_object_of_relationship = $page_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
  $book_object = islandora_object_load($page_object_of_relationship[0]['object']['value']);
  $pid = $page_object->id;
  $title = $book_object->label;
  $pages = islandora_paged_content_get_pages($book_object);
  $pages = array_keys($pages);
  $position = array_search($page_object->id, $pages);

  $results = array();
  $results['page_title'] = $fedora_object->label;
  $results['object_base'] = "islandora/object/$pid";
  $results['position'] = $position;
  $results['pages'] = $pages;
  $results['no_edit'] = !islandora_critical_edition_edit_cwrc_access($fedora_object);
  $results['categories'] = $categories;
  $results['use_dropdown'] = $dropdown;
  // The anno callbacks go to islandora_image_annotation.
  $results['manifest_url'] = "islandora/anno/serve/$pid/Manifest/manifest.xml";
  $results['islandora_post_url'] = "islandora/anno/add_annotation/$pid";
  $results['get_annotation_list_url'] = "islandora/anno/get_urns/$pid";
  // @XXX These links are not dynamic do we need to keep sending them?
  $results['islandora_get_annotation'] = "islandora/anno/get_annotation/";
  $results['islandora_delete_annotation'] = "islandora/anno/delete_annotation/";
  $results['islandora_update_annotation'] = "islandora/anno/update_annotation/";

  echo json_encode($results);
}