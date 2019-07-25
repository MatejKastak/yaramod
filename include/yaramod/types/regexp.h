/**
 * @file src/types/regexp.h
 * @brief Declaration of class Regexp.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#pragma once

#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include <optional_lite/optional.hpp>

#include "yaramod/types/string.h"
#include "yaramod/utils/visitor_result.h"
#include "yaramod/utils/visitor.h"

namespace yaramod {

/**
 * Abstract class representing single unit in the regular expression.
 */
class RegexpUnit
{
public:
	RegexpUnit() {}
	virtual ~RegexpUnit() {}

	virtual std::string getText() const = 0;

	virtual RegexpVisitResult accept(RegexpVisitor* v) = 0;

	virtual double getPower() const = 0;

protected:
	static constexpr double STAR_FACTOR = 40.0; // ?
	static constexpr double PLUS_FACTOR = 42.0; // ?
	static constexpr double OPTIONAL_FACTOR = 0.5; // ?
};

/**
 * Class representing class unit in regular expressions. Class units
 * are enclosed in [] and can be either positive or negative (class starts with symbol ^).
 */
class RegexpClass : public RegexpUnit
{
public:
	RegexpClass(std::string&& characters, bool negative = false) : _characters(characters), _negative(negative) {}

	virtual std::string getText() const
	{
		return '[' + (_negative ? "^" : std::string()) + _characters + ']';
	}

	std::string& getCharacters()
	{
		return _characters;
	}

	bool isNegative() const
	{
		return _negative;
	}

	virtual double getPower() const override
	{
		// TODO: NOT IMPLEMENTED
		return 1.0;
	}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

private:
	std::string _characters; ///< Characters in the class
	bool _negative; ///< Negative class
};

/**
 * Class representing text unit in regular expression.
 * Text unit is sequence of characters with no special meaning.
 */
class RegexpText : public RegexpUnit
{
public:
	RegexpText(const std::string& text) : _text(text) {}
	virtual ~RegexpText() override {}

	virtual std::string getText() const override { return _text; }

	virtual double getPower() const override
	{
		return _text.length();
	}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

private:
	std::string _text; ///< Text
};

/**
 * Class representing unit for any character.
 * This is denoted as character @c . in regular expression.
 */
class RegexpAnyChar : public RegexpText
{
public:
	RegexpAnyChar() : RegexpText(".") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		// TODO: CHECK IMPLEMENTATION
		return static_cast<double>(0xff);
	}

};

/**
 * Class representing unit for word character.
 * This is denoted as character @c \\w in regular expression.
 */
class RegexpWordChar : public RegexpText
{
public:
	RegexpWordChar() : RegexpText("\\w") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}
};

/**
 * Class representing unit for non-word character.
 * This is denoted as character @c \\W in regular expression.
 */
class RegexpNonWordChar : public RegexpText
{
public:
	RegexpNonWordChar() : RegexpText("\\W") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}
};

/**
 * Class representing unit for space character.
 * This is denoted as character @c \\s in regular expression.
 */
class RegexpSpace : public RegexpText
{
public:
	RegexpSpace() : RegexpText("\\s") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}
};

/**
 * Class representing unit for non-space character.
 * This is denoted as character @c \\S in regular expression.
 */
class RegexpNonSpace : public RegexpText
{
public:
	RegexpNonSpace() : RegexpText("\\S") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}
};

/**
 * Class representing unit for digit.
 * This is denoted as character @c \\d in regular expression.
 */
class RegexpDigit : public RegexpText
{
public:
	RegexpDigit() : RegexpText("\\d") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}
};

/**
 * Class representing unit for non-digit.
 * This is denoted as character @c \\D in regular expression.
 */
class RegexpNonDigit : public RegexpText
{
public:
	RegexpNonDigit() : RegexpText("\\D") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}
};

/**
 * Class representing unit for word boundary.
 * This is denoted as character @c \\b in regular expression.
 */
class RegexpWordBoundary : public RegexpText
{
public:
	RegexpWordBoundary() : RegexpText("\\b") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}
};

/**
 * Class representing unit for word boundary.
 * This is denoted as character @c \\B in regular expression.
 */
class RegexpNonWordBoundary : public RegexpText
{
public:
	RegexpNonWordBoundary() : RegexpText("\\B") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}
};

/**
 * Class representing unit for start of line.
 * This is denoted as character @c ^ in regular expression.
 */
class RegexpStartOfLine : public RegexpText
{
public:
	RegexpStartOfLine() : RegexpText("^") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		return 0.0;
	}
};

/**
 * Class representing unit for end of line.
 * This is denoted as character @c $ in regular expression.
 */
class RegexpEndOfLine : public RegexpText
{
public:
	RegexpEndOfLine() : RegexpText("$") {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		return 0.0;
	}
};

/**
 * Abstract class representing unit for operation over certain text in the regular expression.
 * Operation can be either greedy or non-greedy denoted by @c ? at the end of the operation.
 */
class RegexpOperation : public RegexpUnit
{
public:
	virtual ~RegexpOperation() override {}

	virtual std::string getText() const override
	{
		return _operand->getText() + _operation + (_greedy ? std::string() : "?");
	}

	char getOperation() const
	{
		return _operation;
	}

	bool isGreedy() const
	{
		return _greedy;
	}

	std::shared_ptr<RegexpUnit>& getOperand()
	{
		return _operand;
	}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		// TODO: CHECK IMPLEMENTATION
		// This prob needs some factor? No this is parent class, prob never instantiated
		return _operand->getPower();
	}

protected:
	RegexpOperation(char operation, std::shared_ptr<RegexpUnit>&& operand, bool greedy) : _operation(operation), _operand(std::move(operand)), _greedy(greedy) {}

protected:
	char _operation; ///< Operation character
	std::shared_ptr<RegexpUnit> _operand; ///< Operand of the operation
	bool _greedy; ///< Greediness
};

/**
 * Class representing unit for operation of iteration.
 * This is denoted as character @c * in regular expression.
 */
class RegexpIteration : public RegexpOperation
{
public:
	RegexpIteration(std::shared_ptr<RegexpUnit>&& operand, bool greedy) : RegexpOperation('*', std::move(operand), greedy) {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		// TODO: CHECK IMPLEMENTATION
		// This prob needs some factor?
		return _operand->getPower();
	}
};

/**
 * Class representing unit for operation of positive iteration.
 * This is denoted as character @c + in regular expression.
 */
class RegexpPositiveIteration : public RegexpOperation
{
public:
	RegexpPositiveIteration(std::shared_ptr<RegexpUnit>&& operand, bool greedy) : RegexpOperation('+', std::move(operand), greedy) {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		// TODO: CHECK IMPLEMENTATION
		// This prob needs some factor?
		return _operand->getPower();
	}
};

/**
 * Class representing unit for operation of optional occurrence.
 * This is denoted as character @c ? in regular expression.
 */
class RegexpOptional : public RegexpOperation
{
public:
	RegexpOptional(std::shared_ptr<RegexpUnit>&& operand, bool greedy) : RegexpOperation('?', std::move(operand), greedy) {}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		// TODO: CHECK IMPLEMENTATION
		// TODO: ADD OPTIONAL_FACTOR = 0.5
		// This prob needs some factor?
		return 0.5 * _operand->getPower();
	}
};

/**
 * Class representing unit for operation of ranged occurrence.
 * Ranged occurrence may be varying (range unspecified), fixed (N), varying ranged (N-*) and ranged (N-M).
 * This is denoted as @c {} in regular expression.
 */
class RegexpRange : public RegexpOperation
{
public:
	RegexpRange(std::shared_ptr<RegexpUnit>&& operand, std::pair<nonstd::optional<std::uint64_t>, nonstd::optional<std::uint64_t>>&& range, bool greedy)
		: RegexpOperation(' ', std::move(operand), greedy), _range(std::move(range)) {}

	virtual std::string getText() const override
	{
		std::ostringstream ss;
		ss << _operand->getText() << '{';

		if (_range.first && _range.second)
		{
			// If both start and end are defined and they are equal, it is fixed range.
			if (_range.first.value() == _range.second.value())
				ss << _range.first.value();
			else
				ss << _range.first.value() << ',' << _range.second.value();
		}
		else
		{
			if (_range.first)
				ss << _range.first.value();
			ss << ',';
			if (_range.second)
				ss << _range.second.value();
		}

		ss << '}';
		if (!_greedy)
			ss << '?';

		return ss.str();
	}

	std::pair<nonstd::optional<std::uint64_t>, nonstd::optional<std::uint64_t>> getRange()
	{
		return _range;
	}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		// TODO: CHECK IMPLEMENTATION
		// This prob needs some factor?
		// Example:
		// {M,N} => (N - M) + 1 = at most X chars
		// {1,2} => (2 - 1) + 1 = at most 2 chars
		// {M,M} => (M - M) + 1 = at most 1 chars
		// {1,1} => (1 - 1) + 1 = at most 1 chars
		// {N,} => (N - null) + 1 = N * PLUS_FACTOR?
		// {1,} => (1 - null) + 1 = PLUS_FACTOR
		// {0,} => (1 - null) + 1 = STAR_FACTOR
		if (_range.first && _range.second)
		{
			// If both start and end are defined and they are equal, it is fixed range.
			if (_range.first.value() == _range.second.value())
				return _operand->getPower();
			else
				return (_range.second.value() - _range.first.value()) * _operand->getPower();
		}
		else
		{
			if (_range.first)
			{
				switch(_range.first.value()) {
					case 0:
						return STAR_FACTOR * _operand->getPower();
					case 1:
						return PLUS_FACTOR * _operand->getPower();
					default:
						return (STAR_FACTOR / _range.first.value()) * _operand->getPower();
				}
			}
			if (_range.second)
			{
				// TODO: Can this even be triggered? It's not valid regexp
				return 424242.0;
			}
			// TODO: Sensible default?
			return STAR_FACTOR * _operand->getPower();
		}
	}

private:
	std::pair<nonstd::optional<std::uint64_t>, nonstd::optional<std::uint64_t>> _range; ///< Lower and higher bound of the range
};

/**
 * Class representing unit for or operation in regular expressions.
 * This operation cannot be greedy or non-greedy.
 * This is denoted as character @c | in regular expression.
 */
class RegexpOr : public RegexpUnit
{
public:
	RegexpOr(std::shared_ptr<RegexpUnit>&& left, std::shared_ptr<RegexpUnit>&& right) : _left(std::move(left)), _right(std::move(right)) {}

	virtual std::string getText() const override
	{
		return _left->getText() + '|' + _right->getText();
	}

	std::shared_ptr<RegexpUnit>& getLeft()
	{
		return _left;
	}

	std::shared_ptr<RegexpUnit>& getRight()
	{
		return _right;
	}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		// TODO: CHECK IMPLEMENTATION
		// TODO: ADD OPTIONAL_FACTOR = 0.5
		// This prob needs some factor?
		return _left->getPower() + _right->getPower();
	}

private:
	std::shared_ptr<RegexpUnit> _left, _right; ///< Operands
};

/**
 * Class representing unit for group in regular expressions.
 * Groups are parts of regular expression enclosed in @c ().
 */
class RegexpGroup : public RegexpUnit
{
public:
	RegexpGroup(std::shared_ptr<RegexpUnit>&& unit) : _unit(std::move(unit)) {}

	virtual std::string getText() const override
	{
		return '(' + _unit->getText() + ')';
	}

	std::shared_ptr<RegexpUnit>& getUnit()
	{
		return _unit;
	}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		// TODO: CHECK IMPLEMENTATION
		return _unit->getPower();
	}

private:
	std::shared_ptr<RegexpUnit> _unit; ///< Grouped units
};

/**
 * Class representing concatenation in regular expressions.
 * This is not denoted explicitly in the regular expressions,
 * but it happens on the boundaries of two different units.
 */
class RegexpConcat : public RegexpUnit
{
public:
	RegexpConcat(std::vector<std::shared_ptr<RegexpUnit>>&& units) : _units(std::move(units)) {}

	virtual std::string getText() const override
	{
		std::string result;
		for (const auto& unit : _units)
			result += unit->getText();
		return result;
	}

	std::vector<std::shared_ptr<RegexpUnit>>& getUnits()
	{
		return _units;
	}

	virtual RegexpVisitResult accept(RegexpVisitor* v) override
	{
		return v->visit(this);
	}

	virtual double getPower() const override
	{
		double result = 0.0;
		for (const auto& unit : _units)
			result += unit->getPower();
		return result / _units.size();
	}

private:
	std::vector<std::shared_ptr<RegexpUnit>> _units; ///< Concatenated units
};

/**
 * Class representing regular expressions in the strings section
 * of the YARA rules.
 *
 * For example:
 * @code
 * $1 = /md5: [0-9a-zA-Z]{32}/
 * $2 = /state: (on|off)/
 * @endcode
 */
class Regexp : public String
{
public:
	Regexp(std::shared_ptr<RegexpUnit>&& unit) : String(String::Type::Regexp), _unit(std::move(unit)) {}

	virtual std::string getText() const override
	{
		return '/' + getPureText() + '/' + getSuffixModifiers() + getModifiersText();
	}

	virtual std::string getPureText() const override
	{
		return _unit->getText();
	}

	const std::string& getSuffixModifiers() const
	{
		return _suffixMods;
	}

	void setSuffixModifiers(const std::string& suffixMods)
	{
		_suffixMods = suffixMods;
	}

	double getPower() const
	{
		return _unit->getPower();
	}

	const std::shared_ptr<RegexpUnit>& getUnit() const
	{
		return _unit;
	}

private:
	std::shared_ptr<RegexpUnit> _unit; ///< Unit defining other units in regular expression
	std::string _suffixMods; ///< Regular expression suffix modifiers
};

}
