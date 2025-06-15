import { useState } from "react";

import { Card, CardHeader } from "~/components/card";
import { CardBody, CardSubHeader } from "~/components/card/card";
import { useTranslation } from "react-i18next";

import Input from "~/components/input";
import Button from "~/components/button";
import Icon from "~/components/icon";

import styles from './login-form.module.css';

interface LoginFormProps {
  // TODO: May pass username and password setters and getters
  // into this if I'll have a button to switch between this and register.
};

/**
 * Login Form component. Used to log the user in.
 * No actual scheme validation for this currently.
 * 
 * @param
 */
const LoginForm: React.FC<LoginFormProps> = () => {
  const { t } = useTranslation("login");

  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [hovered, setHovered] = useState(false);

  const submitForm = () => {
    // TODO: Do nothing for now
  }
  
  return (
    <Card className={styles.login_form}>
      <div className={styles.login_card_wrapper}>
        <div className={styles.form_header}>
          <CardHeader>{t("form.header.welcome")}</CardHeader>
          <CardSubHeader>{t("form.header.sign-in")}</CardSubHeader>
        </div>
        <CardBody className={styles.form_body}>
          <Input
            placeholder={t("form.body.username")}
            value={username}
            setValue={setUsername}
          />
          <Input
            placeholder={t("form.body.password")}
            type="password"
            value={password}
            setValue={setPassword}
          />
          <Button>
            {t("form.body.sign-in")}
          </Button>
          <div className={styles.divider}>
            <hr className={styles.hr} />
            <CardSubHeader>{t("form.footer.continue")}</CardSubHeader>
            <hr className={styles.hr} />
          </div>
          <Button
            variant="light"
            onMouseOver={() => setHovered(true)}
            onMouseLeave={() => setHovered(false)}
          >
            <Icon
              src={"assets/login/icons/discord.png"}
              hovered={hovered}
            />
            <span>{t("form.footer.discord")}</span>
          </Button>
        </CardBody>
      </div>
    </Card>
  );
}

export default LoginForm;