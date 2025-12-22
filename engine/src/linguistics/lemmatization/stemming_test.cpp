#include "linguistics/lemmatization/stemming.h"

#include <gtest/gtest.h>

using linguistics::StemEn;
using linguistics::StemRu;

TEST(StemRuTest, ShortWordsAreNotChanged) {
  EXPECT_EQ(StemRu("он"), "он");
  EXPECT_EQ(StemRu("дом"), "дом");
  EXPECT_EQ(StemRu("мир"), "мир");
}

TEST(StemRuTest, ReplacesYoWithE) {
  EXPECT_EQ(StemRu("ёлка"), "елк");
  EXPECT_EQ(StemRu("плёнка"), "пленк");
}

TEST(StemRuTest, RemovesNounSuffixes) {
  EXPECT_EQ(StemRu("домов"), "дом");
  EXPECT_EQ(StemRu("машины"), "машин");
  EXPECT_EQ(StemRu("книгах"), "книг");
  EXPECT_EQ(StemRu("полями"), "пол");
}

TEST(StemRuTest, RemovesVerbSuffixes) {
  EXPECT_EQ(StemRu("делала"), "дел");
  EXPECT_EQ(StemRu("писали"), "пис");
  EXPECT_EQ(StemRu("читал"), "чит");
  EXPECT_EQ(StemRu("строить"), "стро");
}

TEST(StemRuTest, RemovesAdjectiveSuffixes) {
  EXPECT_EQ(StemRu("большой"), "больш");
  EXPECT_EQ(StemRu("нового"), "нов");
  EXPECT_EQ(StemRu("синему"), "син");
  EXPECT_EQ(StemRu("быстрые"), "быстр");
}

TEST(StemRuTest, ReturnsOriginalIfNoSuffixMatched) {
  EXPECT_EQ(StemRu("компьютер"), "компьютер");
  EXPECT_EQ(StemRu("алгоритм"), "алгоритм");
}

TEST(StemRuTest, RemovesOnlyOneSuffix) {
  EXPECT_EQ(StemRu("машинами"), "машин");
}

TEST(StemEnTest, ShortWordsAreNotChanged) {
  EXPECT_EQ(StemEn("be"), "be");
  EXPECT_EQ(StemEn("at"), "at");
}

TEST(StemEnTest, Step1aPluralHandling) {
  EXPECT_EQ(StemEn("cats"), "cat");
  EXPECT_EQ(StemEn("classes"), "class");
  EXPECT_EQ(StemEn("ties"), "ti");
  EXPECT_EQ(StemEn("glass"), "glass");
}

TEST(StemEnTest, Step1bEdAndIng) {
  EXPECT_EQ(StemEn("walked"), "walk");
  EXPECT_EQ(StemEn("playing"), "play");
  EXPECT_EQ(StemEn("running"), "run");
}

TEST(StemEnTest, Step1bDoubleConsonantRule) {
  EXPECT_EQ(StemEn("stopped"), "stop");
  EXPECT_EQ(StemEn("planned"), "plan");
}

TEST(StemEnTest, Step1bCvcRule) {
  EXPECT_EQ(StemEn("hopping"), "hop");
  EXPECT_EQ(StemEn("filing"), "file");
}

TEST(StemEnTest, Step1cYToI) {
  EXPECT_EQ(StemEn("happy"), "happi");
  EXPECT_EQ(StemEn("cry"), "cry");
}

TEST(StemEnTest, WordsWithoutStemmingRemainStable) {
  EXPECT_EQ(StemEn("computer"), "computer");
  EXPECT_EQ(StemEn("algorithm"), "algorithm");
}
