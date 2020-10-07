ENTITY miXOR IS
	PORT( A_i : IN BIT; B_i : IN BIT; C_o : OUT BIT);
END miXOR;

ARCHITECTURE estructural OF miXOR IS
	COMPONENT Or2
		PORT( A : IN BIT; B : IN BIT; C : OUT BIT);
	END COMPONENT;
	COMPONENT And2
		PORT( A : IN BIT; B : IN BIT; C : OUT BIT);
	END COMPONENT;
	COMPONENT Inv
		PORT( I : IN BIT; O : OUT BIT);
	END COMPONENT;

	SIGNAL WireLine12, WireLine13, WireLine6, WireLine8: BIT;

	BEGIN
		Or20 : Or2 PORT MAP( WireLine12, WireLine13, C_o);

		And212 : And2 PORT MAP( WireLine6, B_i, WireLine12);

		Inv4 : Inv PORT MAP( B_i, WireLine8);

		And213 : And2 PORT MAP( WireLine8, A_i, WireLine13);

		Inv0 : Inv PORT MAP( A_i, WireLine6);

END estructural;