#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>

#include <check.h>

#include <Elocation.h>

START_TEST(elocation_test_init)
{
   Eina_Bool ret;

   ret = ecore_init();
   fail_if(ret != EINA_TRUE);
   ret = edbus_init();
   fail_if(ret != EINA_TRUE);
   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   // Shutdown relies on a succeeded create_cb right now. Need to be fixed in the code.
   //elocation_shutdown();
   edbus_shutdown();
   ecore_shutdown();
}
END_TEST

Suite *
elocation_suite(void)
{
   Suite *s;
   TCase *tc;

   s = suite_create("Elocation");

   tc = tcase_create("Elocation_Init");
   tcase_add_test(tc, elocation_test_init);
   suite_add_tcase(s, tc);
/*
   tc = tcase_create("Eeze_Udev");
   tcase_add_test(tc, eeze_test_udev_types);
   tcase_add_test(tc, eeze_test_udev_watch);
   tcase_add_test(tc, eeze_test_udev_syspath);
   tcase_add_test(tc, eeze_test_udev_attr);
   suite_add_tcase(s, tc);
*/
   return s;
}

int
main(void)
{
   Suite *s;
   SRunner *sr;
   int failed_count;

   s = elocation_suite();
   sr = srunner_create(s);
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
