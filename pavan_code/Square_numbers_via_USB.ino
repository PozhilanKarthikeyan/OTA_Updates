void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

}

int n=0;

void loop() {
  // put your main code here, to run repeatedly:
  int u = n*n;
  Serial.println(u);
  n+=1;
  delay(1000);

}
