library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


ENTITY FAYARD_Lab3 IS
   PORT(
      clk	  			: in  std_logic;
      reset	  			: in  std_logic;
		push_b			: in std_logic;
		output0 			: out std_logic_vector(7 downto 0);
		output1 			: out std_logic_vector(7 downto 0);
		output2 			: out std_logic_vector(7 downto 0);
		output3 			: out std_logic_vector(7 downto 0);
		output4 			: out std_logic_vector(7 downto 0);
		output5 			: out std_logic_vector(7 downto 0);
		GSENSOR_CS_n 	: out std_logic;			-- _n to indicates that it's active when low
		GSENSOR_SDO 	: out std_logic;
		i2c_SDA 			: inout std_logic ;
		i2c_SCL 			: inout std_logic
      );
END FAYARD_Lab3;


ARCHITECTURE ARCH_FAYARD_Lab3 OF FAYARD_Lab3 IS

component FAYARD_Lab3_sys is
	  port (
			clk_clk                          	: in  std_logic                     	:= 'X'; -- clk
			opencores_i2c_0_export_0_scl_pad_io : inout std_logic                		:= 'X'; -- scl_pad_io
         opencores_i2c_0_export_0_sda_pad_io : inout std_logic                     	:= 'X'; -- sda_pad_io
			pio_0_external_connection_export 	: out std_logic_vector(23 downto 0)				; -- export
			pio_1_external_connection_export    : in    std_logic                     	:= 'X';
			reset_reset_n                    	: in  std_logic                     	:= 'X'  -- reset_n
	  );
end component FAYARD_Lab3_sys;

component bin_to_7seg is
  port (
		input                          	: in  std_logic_vector(3 downto 0);
		output 									: out std_logic_vector(7 downto 0)
  );
end component bin_to_7seg;

signal internal_sig : std_logic_vector(23 downto 0);

begin
	GSENSOR_CS_n 	<= 	'1';
	GSENSOR_SDO 	<= 	'1';
	
    u0 : component FAYARD_Lab3_sys
        port map (
            clk_clk                          	=> clk,
				opencores_i2c_0_export_0_scl_pad_io => i2c_SCL,
				opencores_i2c_0_export_0_sda_pad_io => i2c_SDA,
            pio_0_external_connection_export 	=> internal_sig,
				pio_1_external_connection_export    => push_b,
            reset_reset_n                    	=> reset
    );
  
	sev_seg0: component bin_to_7seg
	port map (
		input                          	=> internal_sig(3 downto 0),                         
		output 									=> output0                  
  );
  
   sev_seg1: component bin_to_7seg
	port map (
		input                          	=> internal_sig(7 downto 4),                         
		output 									=> output1                  
  );
  
   sev_seg2: component bin_to_7seg
	port map (
		input                          	=> internal_sig(11 downto 8),                         
		output 									=> output2                  
  );
  
  	sev_seg3: component bin_to_7seg
	port map (
		input                          	=> internal_sig(15 downto 12),                         
		output 									=> output3                  
  );
  
   sev_seg4: component bin_to_7seg
	port map (
		input                          	=> internal_sig(19 downto 16),                         
		output 									=> output4                  
  );
  
   sev_seg5: component bin_to_7seg
	port map (
		input                          	=> internal_sig(23 downto 20),                         
		output 									=> output5                  
  );
  
  
END ARCH_FAYARD_Lab3;

