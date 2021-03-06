// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include <lib/fit/optional.h>
#include <unittest/unittest.h>

namespace {

struct slot {
    slot(int value = 0)
        : value(value) {
        balance++;
    }
    slot(const slot& other)
        : value(other.value) {
        balance++;
    }
    slot(slot&& other)
        : value(other.value) {
        balance++;
    }

    ~slot() {
        assert(balance > 0);
        assert(value != -1);
        value = -1; // sentinel to catch double-delete
        balance--;
    }

    static int balance; // net constructor/destructor pairings
    int value;

    int get() const { return value; }
    int increment() { return ++value; }

    slot& operator=(const slot& other) = default;
    slot& operator=(slot&& other) = default;

    bool operator==(const slot& other) const { return value == other.value; }
    bool operator!=(const slot& other) const { return value != other.value; }
};
int slot::balance = 0;

bool construct_without_value() {
    BEGIN_TEST;

    fit::optional<slot> opt;
    EXPECT_FALSE(opt.has_value());
    EXPECT_FALSE(!!opt);

    EXPECT_EQ(42, opt.value_or({42}).value);

    opt.reset();
    EXPECT_FALSE(opt.has_value());

    END_TEST;
}

bool construct_with_value() {
    BEGIN_TEST;

    fit::optional<slot> opt({42});
    EXPECT_TRUE(opt.has_value());
    EXPECT_TRUE(!!opt);

    EXPECT_EQ(42, opt.value().value);
    EXPECT_EQ(42, opt.value_or({55}).value);

    EXPECT_EQ(42, opt->get());
    EXPECT_EQ(43, opt->increment());
    EXPECT_EQ(43, opt->get());

    opt.reset();
    EXPECT_FALSE(opt.has_value());

    END_TEST;
}

bool construct_copy() {
    BEGIN_TEST;

    fit::optional<slot> a({42});
    fit::optional<slot> b(a);
    fit::optional<slot> c;
    fit::optional<slot> d(c);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(42, a.value().value);
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(42, b.value().value);
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(d.has_value());

    END_TEST;
}

bool construct_move() {
    BEGIN_TEST;

    fit::optional<slot> a({42});
    fit::optional<slot> b(std::move(a));
    fit::optional<slot> c;
    fit::optional<slot> d(std::move(c));
    EXPECT_FALSE(a.has_value());
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(42, b.value().value);
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(d.has_value());

    END_TEST;
}

bool assign() {
    BEGIN_TEST;

    fit::optional<slot> a({42});
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(42, a.value().value);

    a = {99};
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(99, a.value().value);

    a.reset();
    EXPECT_FALSE(a.has_value());

    a = {55};
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(55, a.value().value);

    a = fit::nullopt;
    EXPECT_FALSE(a.has_value());

    END_TEST;
}

bool assign_copy() {
    BEGIN_TEST;

    fit::optional<slot> a({42});
    fit::optional<slot> b({55});
    fit::optional<slot> c;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(42, a.value().value);
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(55, b.value().value);
    EXPECT_FALSE(c.has_value());

    a = b;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(55, b.value().value);
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(55, b.value().value);

    b = c;
    EXPECT_FALSE(b.has_value());
    EXPECT_FALSE(c.has_value());

    b = a;
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(55, b.value().value);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(55, b.value().value);

    b = b;
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(55, b.value().value);

    c = c;
    EXPECT_FALSE(c.has_value());

    END_TEST;
}

bool assign_move() {
    BEGIN_TEST;

    fit::optional<slot> a({42});
    fit::optional<slot> b({55});
    fit::optional<slot> c;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(42, a.value().value);
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(55, b.value().value);
    EXPECT_FALSE(c.has_value());

    a = std::move(b);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(55, a.value().value);
    EXPECT_FALSE(b.has_value());

    b = std::move(c);
    EXPECT_FALSE(b.has_value());
    EXPECT_FALSE(c.has_value());

    c = std::move(b);
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(b.has_value());

    b = std::move(a);
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(55, b.value().value);
    EXPECT_FALSE(a.has_value());

    b = std::move(b);
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(55, b.value().value);

    a = std::move(a);
    EXPECT_FALSE(a.has_value());

    END_TEST;
}

bool invoke() {
    BEGIN_TEST;

    fit::optional<slot> a({42});
    EXPECT_EQ(42, a->get());
    EXPECT_EQ(43, a->increment());
    EXPECT_EQ(43, (*a).value);

    END_TEST;
}

bool comparisons() {
    BEGIN_TEST;

    fit::optional<slot> a({42});
    fit::optional<slot> b({55});
    fit::optional<slot> c({42});
    fit::optional<slot> d;
    fit::optional<slot> e;

    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a == c);
    EXPECT_FALSE(a == d);
    EXPECT_TRUE(d == e);
    EXPECT_FALSE(d == a);

    EXPECT_FALSE(a == fit::nullopt);
    EXPECT_FALSE(fit::nullopt == a);
    EXPECT_TRUE(a == slot{42});
    EXPECT_TRUE(slot{42} == a);
    EXPECT_FALSE(a == slot{55});
    EXPECT_FALSE(slot{55} == a);
    EXPECT_FALSE(d == slot{42});
    EXPECT_FALSE(slot{42} == d);
    EXPECT_TRUE(d == fit::nullopt);
    EXPECT_TRUE(fit::nullopt == d);

    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a != c);
    EXPECT_TRUE(a != d);
    EXPECT_FALSE(d != e);
    EXPECT_TRUE(d != a);

    EXPECT_TRUE(a != fit::nullopt);
    EXPECT_TRUE(fit::nullopt != a);
    EXPECT_FALSE(a != slot{42});
    EXPECT_FALSE(slot{42} != a);
    EXPECT_TRUE(a != slot{55});
    EXPECT_TRUE(slot{55} != a);
    EXPECT_TRUE(d != slot{42});
    EXPECT_TRUE(slot{42} != d);
    EXPECT_FALSE(d != fit::nullopt);
    EXPECT_FALSE(fit::nullopt != d);

    END_TEST;
}

bool swapping() {
    BEGIN_TEST;

    fit::optional<slot> a({42});
    fit::optional<slot> b({55});
    fit::optional<slot> c;
    fit::optional<slot> d;

    swap(a, b);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(55, a.value().value);
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(42, b.value().value);

    swap(a, c);
    EXPECT_FALSE(a.has_value());
    EXPECT_TRUE(c.has_value());
    EXPECT_EQ(55, c.value().value);

    swap(d, c);
    EXPECT_FALSE(c.has_value());
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(55, d.value().value);

    swap(c, a);
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(a.has_value());

    swap(a, a);
    EXPECT_FALSE(a.has_value());

    swap(d, d);
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(55, d.value().value);

    END_TEST;
}

bool balance() {
    BEGIN_TEST;

    EXPECT_EQ(0, slot::balance);

    END_TEST;
}

// TODO(US-90): Unfortunately fit::optional is not a literal type unlike
// std::optional so expressions involving fit::optional are not constexpr.
// Once we fit that, we should add static asserts to check cases where
// fit::optional wraps a literal type.

} // namespace

BEGIN_TEST_CASE(optional_tests)
RUN_TEST(construct_without_value)
RUN_TEST(construct_with_value)
RUN_TEST(construct_copy)
RUN_TEST(construct_move)
RUN_TEST(assign)
RUN_TEST(assign_copy)
RUN_TEST(assign_move)
RUN_TEST(invoke)
RUN_TEST(comparisons)
RUN_TEST(swapping)
RUN_TEST(balance)
END_TEST_CASE(optional_tests)
