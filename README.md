Integrantes: Paula Campbell López, Marco Mongelli, Matias Weng, Ivan Pistrini

El proyecto consiste en una alarma con un diseño de alfombra. Su objetivo es que el usuario a un cierto horario se despierte y no se vuelva a dormir. Para esto, la alfombra hace sonar una alarma, y hasta que el usuario no apoye sus dos pies sobre la alfombra, la alarma seguirá sonando.

**Utilizamos los siguientes componentes:**
<ul>
<li>ESP32</li>
<li>Pantalla LCD</li>
<li>1 Buzzer</li>
<li>2 LEDs</li>
<li>2 resistencias 470Ω</li>
<li>1 resistencia 2.2 kΩ</li>
</ul>

**Servidor:**
Al ejecutar el programa, el ESP32 se conecta a una red WiFi y hace una solicitud a un sitio web para conseguir la hora en tiempo real. La hora y la fecha se imprimen en la pantalla LCD y se actualiza cada segundo. El ESP32 crea un servidor web asincrónico con una sola ruta: “/crearalarma”, a la que se accede con una solicitud POST. Cuando un cliente web hace una solicitud a la ruta “/crearalarma”, la alfombra configura una alarma usando los datos de los parámetros hora, minuto y repetir que se pasan en la solicitud. Los valores de hora, minuto y repetir son números enteros que determinarán cuándo y cómo debe sonar la alarma.
A su vez, mediante los pines táctiles del microcontrolador ESP32, en paralelo todo el tiempo se sensa si se están tocando dos de estos pines. Esto se implementa con dos placas de aluminio conectadas a dos pines táctiles, que representan lo que luego viene a ser la alfombra (cada placa de aluminio representará un pie). Cuando alguno de estos dos sensores se activa, se enciende un led correspondiente.
Cuando llega la hora en la que la alarma está configurada, mediante un Buzzer, empieza a sonar una melodía. La melodía se repite y sigue sonando hasta que el programa sensa que los pines táctiles fueron activados, es decir, sigue sonando hasta que el usuario pisa sobre las placas de aluminio.

**Cliente**
Para el correcto funcionamiento del dispositivo, es necesario un cliente que configure las alarmas. Esto lo conseguimos creando una aplicación para el celular. La aplicación se encarga de ofrecerle al usuario una interfaz amigable en el cual pueda configurar las alarmas, y luego hacer una solicitud web al servidor creado por el ESP32 enviando los datos de hora y minuto y si la alarma se debe repetir.