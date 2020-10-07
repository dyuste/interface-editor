ENTITY Or2 IS
	PORT( A : IN BIT; B : IN BIT; C : OUT BIT);
END Or2;

ARCHITECTURE functional OF Or2 IS
	BEGIN
		c <= A OR B;
END functional;