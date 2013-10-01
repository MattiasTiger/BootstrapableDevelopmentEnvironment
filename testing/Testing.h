#ifndef _TESTING_H_
#define _TESTING_H_

struct TestCount
{
	int passed;
	int failed;
	TestCount(){passed = 0; failed = 0;}
};
void assert_test(bool cleared, TestCount & count)
{
	if(cleared)
		count.passed++;
	else
		count.failed++;
}
#define INIT_TESTING(name) TestCount _TESTCOUNT; std::cout << ">>TEST " << name << " started\n------------\n";
#define ASSERT_TEST(a,b) {std::cout << ((a == b)?"OK  ":"FAIL") << " - " << #a << " == " << #b <<"\t (== " << a << ")"<< "\n"; assert_test(a == b,_TESTCOUNT);}
#define END_TESTING std::cout << "------------\n<<" << _TESTCOUNT.passed << " of " << _TESTCOUNT.passed+_TESTCOUNT.failed << " passed\n\n";

#endif