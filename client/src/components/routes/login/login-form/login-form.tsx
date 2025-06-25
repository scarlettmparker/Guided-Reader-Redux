import { useState } from "react";
import { UserController } from "~/utils/api";

import { Card, CardHeader } from "~/components/card";
import { CardBody, CardSubHeader } from "~/components/card/card";
import { useTranslation } from "react-i18next";

import Input from "~/components/input";
import Button from "~/components/button";
import Icon from "~/components/icon";

import styles from "./login-form.module.css";

function validateUsername(username: string) {
  return (
    typeof username === "string" &&
    username.length >= 3 &&
    /^[a-zA-Z0-9_]+$/.test(username)
  );
}
function validatePassword(password: string) {
  return typeof password === "string" && password.length >= 8;
}

interface LoginFormProps {
  // TODO: May pass username and password setters and getters
  // into this if I'll have a button to switch between this and register.
}

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
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState(false);

  const submitForm = async (e?: React.FormEvent) => {
    if (e) e.preventDefault();
    setError(null);
    if (!validateUsername(username)) {
      setError(t("form.body.username-error"));
      return;
    }
    if (!validatePassword(password)) {
      setError(t("form.body.password-error"));
      return;
    }
    setLoading(true);
    try {
      await UserController.login(username, password);
      // Optionally: reload user context or redirect
    } catch (err: any) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

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
          {error && <div className={styles.error}>{error}</div>}
          <Button onClick={submitForm} disabled={loading}>
            {loading ? t("form.body.signing-in") : t("form.body.sign-in")}
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
            <Icon src={"assets/login/icons/discord.png"} hovered={hovered} />
            <span>{t("form.footer.discord")}</span>
          </Button>
        </CardBody>
      </div>
    </Card>
  );
};

export default LoginForm;
