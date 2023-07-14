#ifndef MONO_MUSIC_MANAGER__ENUM_OPERATORS
#define MONO_MUSIC_MANAGER__ENUM_OPERATORS


#define ENUM_OPS_PASTE_(identifier1,identifier2) identifier1 ## identifier2
#define ENUM_OPS_PASTE(identifier1,identifier2) ENUM_OPS_PASTE_(identifier1, identifier2)


#define ENUM_DEFINE_OP_(EnumT, op) \
	constexpr EnumT operator op(EnumT lhs, EnumT rhs) \
	{ \
		using T = std::underlying_type_t<EnumT>; \
		return lhs = static_cast<EnumT>(static_cast<T>(lhs) op static_cast<T>(rhs)); \
	}

#define ENUM_DEFINE_OP2_(EnumT, op) \
	ENUM_DEFINE_OP_(EnumT, op) \
	constexpr EnumT operator ENUM_OPS_PASTE(op,=)(EnumT &lhs, EnumT rhs) { return lhs = lhs op rhs; }

#define ENUM_DEFINE_OP3_(EnumT, AnotherT, op) \
	constexpr EnumT operator op(EnumT lhs, AnotherT rhs) \
	{ \
		using T = std::underlying_type_t<EnumT>; \
		return lhs = static_cast<EnumT>(static_cast<T>(lhs) op rhs); \
	} \
	constexpr EnumT operator ENUM_OPS_PASTE(op,=)(EnumT &lhs, EnumT rhs) { return lhs = lhs op rhs; }




#define ENUM_DEFINE_BITWISE_NOT(EnumT) \
	constexpr EnumT operator~(EnumT rhs) \
	{ \
		using T = std::underlying_type_t<EnumT>; \
		return static_cast<EnumT>(~static_cast<T>(rhs)); \
	}
#define ENUM_DEFINE_BITWISE_OR(EnumT) ENUM_DEFINE_OP2_(EnumT, |)
#define ENUM_DEFINE_BITWISE_AND(EnumT) ENUM_DEFINE_OP2_(EnumT, &)
#define ENUM_DEFINE_BITWISE_XOR(EnumT) ENUM_DEFINE_OP2_(EnumT, ^)
#define ENUM_DEFINE_MATH_ADD(EnumT) ENUM_DEFINE_OP2_(EnumT, +)
#define ENUM_DEFINE_MATH_SUB(EnumT) ENUM_DEFINE_OP2_(EnumT, -)
#define ENUM_DEFINE_MATH_MUL(EnumT) ENUM_DEFINE_OP2_(EnumT, *)
#define ENUM_DEFINE_MATH_DIV(EnumT) ENUM_DEFINE_OP2_(EnumT, /)
#define ENUM_DEFINE_SHIFT_LEFT(EnumT) ENUM_DEFINE_OP3_(EnumT, int, <<)
#define ENUM_DEFINE_SHIFT_RIGHT(EnumT) ENUM_DEFINE_OP3_(EnumT, int, >>)

#define ENUM_DEFINE_BITWISE_OPS(EnumT) \
	ENUM_DEFINE_BITWISE_NOT(EnumT) \
	ENUM_DEFINE_BITWISE_OR(EnumT) \
	ENUM_DEFINE_BITWISE_AND(EnumT) \
	ENUM_DEFINE_BITWISE_XOR(EnumT)

#define ENUM_DEFINE_SHIFT_OPS(EnumT) \
	ENUM_DEFINE_SHIFT_LEFT(EnumT) \
	ENUM_DEFINE_SHIFT_RIGHT(EnumT)

#define ENUM_DEFINE_MATH_OPS(EnumT) \
	ENUM_DEFINE_MATH_ADD(EnumT) \
	ENUM_DEFINE_MATH_SUB(EnumT) \
	ENUM_DEFINE_MATH_MUL(EnumT) \
	ENUM_DEFINE_MATH_DIV(EnumT)

#endif /* MONO_MUSIC_MANAGER__ENUM_OPERATORS */
