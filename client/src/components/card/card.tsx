import { Component, JSX } from "solid-js";
import styles from "./card.module.css";

interface CardProps {
  children: JSX.Element;
  class?: string;
};

export const Card: Component<CardProps> = (props) => {
  const { children, class: class_ = "" } = props;
  return (
    <div class={`${styles.card} ${class_}`}>
      {children}
    </div>
  );
};

interface CardHeaderProps {
  children: JSX.Element;
  class?: string;
};

export const CardHeader: Component<CardHeaderProps> = (props) => {
  const { children, class: class_ = "" } = props;
  return (
    <div class={`${styles.card_header} ${class_}`}>
      {children}
    </div>
  );
};

interface CardBodyProps {
  children: JSX.Element;
  class?: string;
};

export const CardBody: Component<CardBodyProps> = (props) => {
  const { children, class: class_ = "" } = props;
  return (
    <div class={`${styles.card_body} ${class_}`}>
      {children}
    </div>
  );
};

interface CardFooterProps {
  children: JSX.Element;
  class?: string;
};

export const CardFooter: Component<CardFooterProps> = (props) => {
  const { children, class: class_ = "" } = props;
  return (
    <div class={`${styles.card_footer} ${class_}`}>
      {children}
    </div>
  );
};