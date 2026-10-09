#include "plf_tools.h"

#include <cstdio> // log redirection
#include <cstdlib> // abort
#include <string>

#ifdef PLF_MOVE_SEMANTICS_SUPPORT
	#include <utility> // std::move
#endif

#include "plf_stack.h"




#ifdef PLF_EXCEPTIONS_SUPPORT
	// Copy construction throws once copies_until_throw reaches 0. There is no noexcept move constructor, so stack::append copies it:
	int copies_until_throw = -1;

	struct throwing_copy
	{
		int value;

		throwing_copy(const int number): value(number) {}

		throwing_copy(const throwing_copy &source): value(source.value)
		{
			if (copies_until_throw >= 0 && copies_until_throw-- == 0) throw 1;
		}
	};
#endif



// Pops elements and checks that they come out as last, last - 1, ... first:
template <class stack_type>
bool pops_in_order(stack_type &the_stack, const int last, const int first)
{
	for (int expected = last; expected != first - 1; --expected)
	{
		if (the_stack.empty() || the_stack.top() != expected) return false;
		the_stack.pop();
	}

	return true;
}



void title1(const char *title_text)
{
	printf("\n\n\n*** %s ***\n", title_text);
	printf("===========================================\n\n\n");
}

void title2(const char *title_text)
{
	printf("\n\n--- %s ---\n\n", title_text);
}


void failpass(const char *test_type, bool condition)
{
	printf("%s: ", test_type);

	if (condition)
	{
		printf("Pass\n");
	}
	else
	{
		printf("Fail\n");
		getchar();
		abort();
	}
}




#ifdef PLF_VARIADICS_SUPPORT
	struct perfect_forwarding_test
	{
		const bool success;

		perfect_forwarding_test(int && /*perfect1*/, int& perfect2)
			: success(true)
		{
			perfect2 = 1;
		}

		template <typename T, typename U>
		perfect_forwarding_test(T&& /*imperfect1*/, U&& /*imperfect2*/)
			: success(false)
		{}
	};
#endif


#if defined(PLF_TYPE_TRAITS_SUPPORT) && (__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
	struct non_copyable_non_movable
	{
		int value;

		non_copyable_non_movable(const int v) : value(v) {}
		non_copyable_non_movable(const non_copyable_non_movable &) = delete;
		non_copyable_non_movable & operator = (const non_copyable_non_movable &) = delete;
	};
#endif



int main()
{
	freopen("error.log","w", stderr);

	using namespace std;
	using namespace plf;


	unsigned int looper = 0;


	while (++looper != 50)
	{
		{
			title1("Test basics");

			stack<unsigned int> i_stack(50);

			for (unsigned int temp = 0; temp != 250000; ++temp)
			{
				i_stack.push(10);
			}

			failpass("Multipush test", i_stack.size() == 250000);

			stack<unsigned int> i_stack2;
			i_stack2 = i_stack;

			stack<unsigned int> i_stack3(i_stack);

			failpass("Copy constructor test", i_stack3.size() == 250000);

			stack<unsigned int> i_stack6(i_stack, i_stack3.get_allocator());

			failpass("Allocator-extended copy constructor test", i_stack6.size() == 250000);

			i_stack3.reserve(400000);

			failpass("Reserve test", i_stack3.size() == 250000);


			stack<unsigned int> i_stack7(50, 50);

			for (unsigned int temp = 0; temp != 449; ++temp)
			{
				i_stack7.push(10);
			}

			failpass("Max limit test", i_stack7.capacity() == 450);

			i_stack7.reshape(100, 100);

			failpass("Reshape test", i_stack7.capacity() == 500);


			#ifdef PLF_MOVE_SEMANTICS_SUPPORT
				stack<unsigned int> i_stack4;
				i_stack4 = std::move(i_stack3);
				failpass("Move equality operator test", i_stack2 == i_stack4);
				stack<unsigned int> i_stack5(std::move(i_stack4), i_stack3.get_allocator());

				failpass("Allocator-extended move-construct test", i_stack5.size() == 250000);

				i_stack3 = std::move(i_stack5);

			#else
				failpass("Equality operator test", i_stack2 == i_stack3);
			#endif

			failpass("Copy test", i_stack2.size() == 250000);
			failpass("Equality operator test 2", i_stack == i_stack2);

			i_stack2.push(5);

 			failpass("Inequality operator test", i_stack != i_stack2);

			i_stack2.swap(i_stack3);

			failpass("Swap test", i_stack2.size() == i_stack3.size() - 1);

			swap(i_stack2, i_stack3);

			failpass("Swap test 2", i_stack3.size() == i_stack2.size() - 1);

			failpass("max_size() test", i_stack2.max_size() > i_stack2.size());


			unsigned int total = 0;

			const unsigned int temp_capacity = static_cast<unsigned int>(i_stack.capacity());

			for (unsigned int temp = 0; temp != 200000; ++temp)
			{
				total += i_stack.top();
				i_stack.pop();
			}

			failpass("Multipop test", i_stack.size() == 50000);
			failpass("top() test", total == 2000000);

			i_stack.shrink_to_fit();

			failpass("shrink_to_fit() test", temp_capacity != i_stack.capacity());


			do
			{
				if ((rand() & 3) == 0)
				{
					i_stack.push(10);
				}
				else
				{
					i_stack.pop();
				}
			} while (!i_stack.empty());;

			failpass("Randomly pop/push till empty test", i_stack.empty());

			#ifdef PLF_VARIADICS_SUPPORT
				i_stack.emplace(20);
				failpass("Emplace test", i_stack.size() == 1);
			#endif
		}

		{
			title2("Stack Special Case Tests");

			stack<int> i_stack(50, 100);

			for (int temp = 0; temp != 256; ++temp)
			{
				i_stack.push(10);
			}

			stack<int> i_stack_copy(i_stack);

			int temp2 = 0;

			for (int temp = 0; temp != 256; ++temp)
			{
				temp2 += i_stack_copy.top();
				i_stack_copy.pop();
			}

			failpass("Stack copy special case test", temp2 == 2560);
		}


		{
 			title2("stack expansion test");

 			stack<int> i_stack;
			int push_total = 0, pop_total = 0;

			for (int counter = 0; counter != 500; ++counter)
			{
				i_stack.push(counter);
				push_total += counter;
			}


 			do
 			{
				pop_total += i_stack.top();
				i_stack.pop();
 			} while (!i_stack.empty());


			failpass("stack expansion test 1", pop_total == push_total);


			push_total = 0;
			pop_total = 0;


			for (int counter = 0; counter != 50; ++counter)
			{
				i_stack.push(counter);
				push_total += counter;
			}


 			do
 			{
 				if ((rand() & 3) == 0)
 				{
 					pop_total += i_stack.top();
					i_stack.pop();
 				}
 				else
 				{
 					i_stack.push(10);
					push_total += 10;
 				}
 			} while (!i_stack.empty() && i_stack.capacity() < 5000);



 			do
 			{
 				if ((rand() & 3) == 0)
 				{
 					i_stack.push(10);
					push_total += 10;
 				}
 				else
 				{
 					pop_total += i_stack.top();
					i_stack.pop();
 				}
 			} while (!i_stack.empty());


 			failpass("random stack expansion test", pop_total == push_total);
		}



		#ifdef PLF_VARIADICS_SUPPORT
		{
			title2("Perfect Forwarding tests");

			stack<perfect_forwarding_test> pf_stack;

			int lvalue = 0;
			int &lvalueref = lvalue;

			pf_stack.emplace(7, lvalueref);

			failpass("Perfect forwarding test", pf_stack.top().success);
			failpass("Perfect forwarding test 2", lvalueref == 1);
		}
		#endif


		#if defined(PLF_TYPE_TRAITS_SUPPORT) && defined(PLF_VARIADICS_SUPPORT) && defined(PLF_EXCEPTIONS_SUPPORT) && (__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)) // before C++17, reshape() cannot compile for this type because consolidate() is instantiated without if constexpr
		{
			title2("Reshape non-copyable non-movable type test");

			stack<non_copyable_non_movable> ncm_stack;
			ncm_stack.emplace(1);

			const stack<non_copyable_non_movable>::size_type capacity_before = ncm_stack.capacity();
			bool threw_length_error = false;

			try
			{
				ncm_stack.reshape(2, 2); // the existing block is outside these limits and cannot be consolidated
			}
			catch (const std::length_error &)
			{
				threw_length_error = true;
			}

			failpass("Reshape non-copyable non-movable type throws length_error", threw_length_error);
			failpass("Reshape non-copyable non-movable type leaves stack unchanged", ncm_stack.size() == 1 && ncm_stack.top().value == 1 && ncm_stack.capacity() == capacity_before);

			// The rejected limits must not be kept: the next block should not be capped at 2 elements
			while (ncm_stack.size() != capacity_before)
			{
				ncm_stack.emplace(1);
			}

			ncm_stack.emplace(1);
			failpass("Reshape non-copyable non-movable type keeps the old block capacity limits", ncm_stack.capacity() - capacity_before > 2);
		}
		#endif


		{
			title2("append tests");

			{
				stack<int> stack1, stack2;

				for(int number = 0; number != 20; ++number)
				{
					stack1.push(number);
					stack2.push(number + 20);
				}

				stack1.append(stack2);

				int check_number = 0;

				while(!stack1.empty())
				{
					check_number += stack1.top();
					stack1.pop();
				}

				failpass("Small append test 1", check_number == 780);
			}


			{
				stack<unsigned int> stack1, stack2;

				for(unsigned int number = 0; number != 100000; ++number)
				{
					stack1.push(number);
					stack2.push(number + 100000);
				}

				stack1.append(stack2);

				unsigned int check_number = 0;

				while(!stack1.empty())
				{
					check_number += stack1.top();
					stack1.pop();
				}

				failpass("Large append test 1", check_number == 2820030816u);
			}



			{
				stack<int> stack1, stack2;

				for(int number = 150; number != 250; ++number)
				{
					stack1.push(number);
				}


				for(int number = 0; number != 150; ++number)
				{
					stack2.push(number);
				}


				stack1.append(stack2);

				int check_number = 0;

				while(!stack1.empty())
				{
					check_number += stack1.top();
					stack1.pop();
				}


				failpass("Unequal size append test 1", check_number == 31125);
			}


			{
				stack<int> stack1(3, 8), stack2(3, 3);

				for (int number = 1; number != 5; ++number)
				{
					stack1.push(number); // blocks of 3 and 6: the top block is not full
				}

				for (int number = 10; number != 16; ++number)
				{
					stack2.push(number);
				}

				stack1.append(stack2);

				failpass("Append onto a partly-filled block capacity test", stack1.capacity() >= stack1.size() && stack1.size() == 10 && stack2.empty());
				failpass("Append onto a partly-filled block order test", pops_in_order(stack1, 15, 10) && pops_in_order(stack1, 4, 1) && stack1.empty());
			}


			#ifdef PLF_MOVE_SEMANTICS_SUPPORT
			{
				stack<std::string> stack1(3, 8), stack2(3, 3); // std::string is nothrow-movable, so the source's elements are moved down into the free locations

				for (int number = 1; number != 5; ++number)
				{
					stack1.push(std::string(30, char('a' + number)));
				}

				for (int number = 10; number != 16; ++number)
				{
					stack2.push(std::string(30, char('a' + number)));
				}

				stack1.append(stack2);

				bool correct = stack1.size() == 10 && stack2.empty() && stack1.capacity() >= stack1.size();

				for (int number = 15; correct && number != 9; --number)
				{
					correct = stack1.top() == std::string(30, char('a' + number));
					stack1.pop();
				}

				for (int number = 4; correct && number != 0; --number)
				{
					correct = stack1.top() == std::string(30, char('a' + number));
					stack1.pop();
				}

				failpass("Append onto a partly-filled block with a non-trivial type test", correct && stack1.empty());
			}
			#endif


			{
				stack<int> stack1(3, 3), stack2(3, 3);

				for (int number = 1; number != 4; ++number)
				{
					stack1.push(number); // the top block is full, so the source's blocks are linked
				}

				for (int number = 10; number != 16; ++number)
				{
					stack2.push(number);
				}

				const stack<int>::size_type expected_capacity = stack1.capacity() + stack2.capacity();
				stack1.append(stack2);

				failpass("Append onto a full block capacity test", stack1.capacity() == expected_capacity && stack1.size() == 9);
				failpass("Append onto a full block order test", pops_in_order(stack1, 15, 10) && pops_in_order(stack1, 3, 1) && stack1.empty());
			}


			#ifdef PLF_EXCEPTIONS_SUPPORT
			{
				stack<int> stack1(3, 3), stack2(10, 10);
				stack1.push(1);

				for (int number = 10; number != 15; ++number)
				{
					stack2.push(number);
				}

				bool threw_length_error = false;

				try
				{
					stack1.append(stack2); // stack2's block is larger than stack1's maximum block capacity
				}
				catch (const std::length_error &)
				{
					threw_length_error = true;
				}

				failpass("Append with incompatible block capacities throws length_error", threw_length_error);
				failpass("Append with incompatible block capacities leaves both stacks unchanged", pops_in_order(stack1, 1, 1) && stack1.empty() && pops_in_order(stack2, 14, 10) && stack2.empty());
			}


			{
				stack<throwing_copy> stack1(3, 8), stack2(3, 3);

				for (int number = 1; number != 5; ++number)
				{
					stack1.push(throwing_copy(number)); // the top block is not full, so the elements are copied
				}

				for (int number = 10; number != 16; ++number)
				{
					stack2.push(throwing_copy(number));
				}

				bool threw = false;
				copies_until_throw = 3;

				try
				{
					stack1.append(stack2);
				}
				catch (int)
				{
					threw = true;
				}

				copies_until_throw = -1;

				bool unchanged = stack1.size() == 4 && stack2.size() == 6;

				for (int number = 4; unchanged && number != 0; --number)
				{
					unchanged = stack1.top().value == number;
					stack1.pop();
				}

				for (int number = 15; unchanged && number != 9; --number)
				{
					unchanged = stack2.top().value == number;
					stack2.pop();
				}

				failpass("Append with a throwing copy leaves both stacks unchanged", threw && unchanged);
			}
			#endif
		}



		{
			title1("Iterator tests");

			stack<int> istack;
			int total = 0;

			for (int temp = 0; temp != 1000; ++temp)
			{
				istack.push(1);
				++total;
			}

			int number_of_elements = static_cast<int>(istack.size());

			for (stack<int>::iterator current = istack.begin(); current != istack.end(); ++current)
			{
				total -= *current;
				--number_of_elements;
			}

			failpass("Iterator test 1", total == 0 && number_of_elements == 0);

			istack.pop();
			istack.pop();
			istack.pop();
			istack.pop();
			istack.pop();

			number_of_elements = static_cast<int>(istack.size());

			for (stack<int>::iterator current = istack.begin(); current != istack.end(); ++current)
			{
				--number_of_elements;
			}

			failpass("Iterator test 2", number_of_elements == 0);

			istack.push(1);
			istack.push(1);

			number_of_elements = static_cast<int>(istack.size());

			for (stack<int>::reverse_iterator current = istack.rbegin(); current != istack.rend(); ++current)
			{
				--number_of_elements;
			}

			failpass("Reverse Iterator test 1", number_of_elements == 0);

		}
	}

	title1("Test Suite PASS - Press ENTER to Exit");
	getchar();

	return 0;
}
