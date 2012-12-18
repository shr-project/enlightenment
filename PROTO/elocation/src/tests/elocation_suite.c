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

   elocation_shutdown();
   edbus_shutdown();
   ecore_shutdown();
}
END_TEST

START_TEST(elocation_test_address_object)
{
   Eina_Bool ret;
   Elocation_Address *address = NULL;

   ret = ecore_init();
   fail_if(ret != EINA_TRUE);
   ret = edbus_init();
   fail_if(ret != EINA_TRUE);
   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   address = elocation_address_new();
   fail_if(address == NULL);

   elocation_address_free(address);

   elocation_shutdown();
   edbus_shutdown();
   ecore_shutdown();
}
END_TEST

START_TEST(elocation_test_position_object)
{
   Eina_Bool ret;
   Elocation_Position *position = NULL;

   ret = ecore_init();
   fail_if(ret != EINA_TRUE);
   ret = edbus_init();
   fail_if(ret != EINA_TRUE);
   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   position = elocation_position_new();
   fail_if(position == NULL);

   elocation_position_free(position);

   elocation_shutdown();
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

   tc = tcase_create("Elocation_Objects");
   tcase_add_test(tc, elocation_test_address_object);
   tcase_add_test(tc, elocation_test_position_object);
   suite_add_tcase(s, tc);

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
