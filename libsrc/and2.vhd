ENTITY And2 IS
	PORT( A : IN BIT; B : IN BIT; C : OUT BIT);
END And2;

ARCHITECTURE functional OF And2 IS
	BEGIN
		C <= A AND B;
END functional;