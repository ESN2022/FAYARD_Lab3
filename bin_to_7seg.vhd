library ieee;
use ieee.std_logic_1164.all;

entity bin_to_7seg is
  port (input: in std_logic_vector(3 downto 0);
        output: out std_logic_vector(7 downto 0));
end bin_to_7seg;

architecture arch_bin_to_7seg of bin_to_7seg is
begin
	output <= 	"11000000" when input="0000" else
					"11111001" when input="0001" else
					"10100100" when input="0010" else
					"10110000" when input="0011" else
					"10011001" when input="0100" else
					"10010010" when input="0101" else
					"10000010" when input="0110" else
					"11111000" when input="0111" else
					"10000000" when input="1000" else
					"10010000" when input="1001" else
					"00000000";
end arch_bin_to_7seg;


