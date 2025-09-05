/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a2c6d79d299cd37711158413ec1e33242e405370 */

static zend_class_entry *register_class_pocketmine_world_format_PalettedBlockArrayLoadException(zend_class_entry *class_entry_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "pocketmine\\world\\format", "PalettedBlockArrayLoadException", NULL);
#if (PHP_VERSION_ID >= 80400)
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_RuntimeException, 0);
#else
	class_entry = zend_register_internal_class_ex(&ce, class_entry_RuntimeException);
#endif

	return class_entry;
}
