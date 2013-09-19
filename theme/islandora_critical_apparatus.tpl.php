<?php
/**
 * @file
 * template for critical apparatus
 *
 */
$object = $variables['islandora_object'];
module_load_include('inc', 'islandora', 'includes/breadcrumb');
drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
drupal_set_title($object->label);
$datastreams = $variables['datastreams'];
$dsids = array_keys($datastreams);
$dsids_to_display = array();
foreach ($dsids as $dsid) {
  if ($object[$dsid]) {
    $dsids_to_display[] = $dsid;
  }
}
?>
<script>
  $(function() {
    $("#tabs").tabs();
  });
</script>

<div id="tabs">
  <ul>
    <?php foreach ($dsids_to_display as $dsid): ?>
      <li><a href="#<?php print $dsid; ?>"><?php print $datastreams[$dsid]; ?></a></li>
    <?php endforeach; ?>
  </ul>
  <?php foreach ($dsids_to_display as $dsid): ?>
    <div id="<?php print $dsid; ?>"">
      <p><?php print $object[$dsid]->content; ?></p>
    </div>
  <?php endforeach; ?>
</div>